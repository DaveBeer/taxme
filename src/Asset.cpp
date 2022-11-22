#include <algorithm>
#include <iterator>
#include <queue>
#include <iostream>
#include <iomanip>
#include <limits>
#include "common.h"
#include "Asset.h"

using BuysQueue = std::queue<Record*>;

static CSVLine GetCSVLine(std::string const& line);
static Label ParseType(std::string const& s, LabelsConfig const& config);
static double ParseDouble(std::string const& s, std::string const& context);
static bool ParseIsTaxed(std::string const& s, LabelsConfig const& config);

static double SellFromBuysAndGetCosts(double sellQuantity, BuysQueue& buys);
static bool CheckForInsignificantRemainingSellQuantityAndReportWarning(double sellQuantity);
static double GetPrice(InputData const& data);

static void WriteCSVLine(CSVLine const& line, std::ostream& file);
static std::string DoubleToString(double d);

Asset::Asset(LabelsConfig const& config, std::ifstream& file)
{
    LoadCSVHeader(config, file);
    LoadCSVData(config, file);
}

void Asset::LoadCSVHeader(LabelsConfig const& config, std::ifstream& file)
{
    std::string line;
    std::getline(file, line);
    header = GetCSVLine(line);

    if (header.empty())
    {
        throw std::runtime_error("Error: asset csv file has no header");
    }

    ParseHeader(config);
}

void Asset::ParseHeader(LabelsConfig const& config)
{
    auto const labelsToDefine = {
        Label::Date, Label::Type, Label::UnitPrice, Label::Quantity,
        Label::ClosedPositionQuantity, Label::Gain, Label::IsTaxed};

    for (auto const label : labelsToDefine)
    {
        auto const value = config.label2Value.at(label);
        auto const it = std::find(std::begin(header), std::end(header), value);

        if (it == std::end(header))
        {
            throw std::runtime_error("Error: asset csv file is missing column " + value);
        }

        label2Index[label] = std::distance(std::begin(header), it);
    }
}

void Asset::LoadCSVData(LabelsConfig const& config, std::ifstream& file)
{
    std::string line;

    while (std::getline(file, line))
    {
        Record r;
        r.row = GetCSVLine(line);

        if (r.row.size() != header.size())
        {
            throw std::runtime_error("Error: asset csv file has data of inconsistent size");
        }

        ParseInputs(config, r);

        records.push_back(r);
    }
}

void Asset::ParseInputs(LabelsConfig const& config, Record& r) const
{
    r.inputs.year = ParseYear(r.row[label2Index.at(Label::Date)], "asset csv file");
    r.inputs.type = ParseType(r.row[label2Index.at(Label::Type)], config);
    r.inputs.unitPrice = ParseDouble(r.row[label2Index.at(Label::UnitPrice)], "asset csv file unit price");
    r.inputs.quantity = ParseDouble(r.row[label2Index.at(Label::Quantity)], "asset csv file quantity");
    r.inputs.isTaxed = ParseIsTaxed(r.row[label2Index.at(Label::IsTaxed)], config);
}

void Asset::ComputeOutputs(int const yearReportingTaxFor, bool const commitTax)
{
    BuysQueue fifoBuys; // points to records which no longer change at this point

    for (Record& r : records)
    {
        if (r.inputs.type == Label::TypeBuy)
        {
            r.outputs.closedPositionQuantity = 0;
            fifoBuys.push(&r);
        }
        else // TypeSell
        {
            auto const costs = SellFromBuysAndGetCosts(r.inputs.quantity, fifoBuys);
            r.outputs.gain = GetPrice(r.inputs) - costs;
            r.outputs.isTaxed = r.inputs.isTaxed || (commitTax && r.inputs.year == yearReportingTaxFor);
        }
    }
}

FinancialOutcome Asset::GetFinancialOutcome(int const year) const
{
    FinancialOutcome result = {0, 0};

    for (auto const& r : records)
    {
        if (r.inputs.year == year && r.inputs.type == Label::TypeSell)
        {
            auto const income = GetPrice(r.inputs);
            result.income += income;
            result.costs += income - r.outputs.gain;
        }
    }

    return result;
}

void Asset::WriteUpdatedFile(LabelsConfig const& config, std::ofstream& file) const
{
    WriteCSVLine(header, file);

    for (auto const& r : records)
    {
        WriteCSVLine(GetUpdatedCSVLine(r, config), file);
    }

    if (!file)
    {
        throw std::runtime_error("Error: failed writing updated asset csv file");
    }
}

CSVLine Asset::GetUpdatedCSVLine(Record const &r, LabelsConfig const& config) const
{
    auto row = r.row;
    auto& closedPositionQuantity = row[label2Index.at(Label::ClosedPositionQuantity)];
    auto& gain = row[label2Index.at(Label::Gain)];
    auto& isTaxed = row[label2Index.at(Label::IsTaxed)];
    auto const& na = config.label2Value.at(Label::NotAvailable);

    if (r.inputs.type == Label::TypeBuy)
    {
        closedPositionQuantity = DoubleToString(r.outputs.closedPositionQuantity);
        gain = na;
        isTaxed = na;
    }
    else
    {
        closedPositionQuantity = na;
        gain = DoubleToString(r.outputs.gain);
        isTaxed = config.label2Value.at((r.outputs.isTaxed) ? Label::Yes : Label::No);
    }

    return row;
}

static CSVLine GetCSVLine(std::string const& line)
{
    CSVLine result;
    std::istringstream iss(line);
    std::string cell;

    while (std::getline(iss, cell, ','))
    {
        result.push_back(cell);
    }

    if (!line.empty() && line.back() == ',')
    {
        result.push_back("");
    }

    return result;
}

static Label ParseType(std::string const& s, LabelsConfig const& config)
{
    try
    {
        auto const type = config.value2Label.at(s);

        if (type != Label::TypeBuy && type != Label::TypeSell)
        {
            throw std::out_of_range("");
        }

        return type;
    }
    catch (std::out_of_range const&)
    {
        throw std::runtime_error("Error: asset csv file contains unknown type value");
    }
}

static double ParseDouble(std::string const& s, std::string const& context)
{
    std::istringstream iss(s);
    double d;
    std::string acceptAny;

    if (!(iss >> d) || (iss >> acceptAny)) // parsing double failed or contains more characters after double
    {
        throw std::runtime_error("Error: failed to parse number from " + context);
    }

    if (d < 0)
    {
        throw std::runtime_error("Error: negative number in " + context);
    }

    return d;
}

static bool ParseIsTaxed(std::string const& s, LabelsConfig const& config)
{
    try
    {
        auto const label = config.value2Label.at(s);
        return label == Label::Yes;
    }
    catch (std::out_of_range const&)
    {
        return false; // fill empty or correct malformed values to No
    }
}

static double SellFromBuysAndGetCosts(double const sellQuantity, BuysQueue& buys)
{
    auto costs = 0.0;
    auto remainsToSell = sellQuantity;

    while (remainsToSell != 0)
    {
        if (buys.empty())
        {
            if (CheckForInsignificantRemainingSellQuantityAndReportWarning(remainsToSell))
            {
                break; // we're done, ignore loss of significance error
            }

            throw std::runtime_error("Error: asset csv file has more sold quantity than bought");
        }

        auto& buy = *buys.front();
        auto const buyQuantityLeft = buy.inputs.quantity - buy.outputs.closedPositionQuantity;
        double sellFromThisBuy;

        if (remainsToSell >= buyQuantityLeft)
        {
            sellFromThisBuy = buyQuantityLeft;
            buys.pop();
        }
        else
        {
            sellFromThisBuy = remainsToSell;
        }

        buy.outputs.closedPositionQuantity += sellFromThisBuy;
        costs += buy.inputs.unitPrice * sellFromThisBuy;
        remainsToSell -= sellFromThisBuy;

        if (remainsToSell < 0 || buy.outputs.closedPositionQuantity > buy.inputs.quantity)
        {
            // ideally assets would be recorded in smallest units so quantities would be integers
            // and we wouldn't have to deal with loss of significance on doubles subtraction
            throw std::runtime_error("Logical error: inaccuracy on operation with doubles");
        }
    }

    return costs;
}

static bool CheckForInsignificantRemainingSellQuantityAndReportWarning(double const sellQuantity)
{
    if (sellQuantity < 1e-8)
    {
        /* I have yet to see exchange which supports more than 8 decimals on any asset. Since
           operations with quantity are additions and subtractions only, it can never fall to
           lower decimal in pure math. Whenever there is remaining sell quantity bellow 1e-8,
           it is result of loss of significance on doubles subtraction, given the above
           assumption holds. This already happened in real data, so implementing workaround
           with warning instead of just fail detection.
        */

        auto const origPrecision = std::cout.precision();

        std::cout << "Warning: there is positive remaining sold quantity (likely loss of significance error): "
                  << std::setprecision(std::numeric_limits<double>::digits10)
                  << sellQuantity << std::setprecision(origPrecision) << std::endl;

        return true;
    }

    return false;
}

static double GetPrice(InputData const& data)
{
    return data.unitPrice * data.quantity;
}

static void WriteCSVLine(CSVLine const& line, std::ostream& file)
{
    file << line.front(); // guaranteed it exists by checks when loading CSV file

    for (auto it = std::begin(line) + 1; it != std::end(line); ++it)
    {
        file << ',' << *it;
    }

    file << '\n';
}

static std::string DoubleToString(double const d)
{
    std::ostringstream oss;
    oss << std::setprecision(std::numeric_limits<double>::digits10) << d;

    return oss.str(); 
}
