#include <cstdlib>
#include <iostream>
#include <ios>
#include "common.h"
#include "Asset.h"

struct Inputs
{
    int year;
    bool commitTax;
    LabelsConfig labelsConfig;
    std::vector<std::string> assetFileNames;
};

void CheckFile(std::ios const& f, std::string const& description);

Inputs ParseArguments(int argc, char* argv[]);
bool ParseCommitTax(std::string const& input);
void LoadLabels(std::string fileName, LabelsConfig& config);
void LoadLabel(std::string const& line, LabelsConfig& config);
void CheckAllLabelsDefined(LabelsConfig const& config);

void ProcessAssets(Inputs const& inputs);
FinancialOutcome ProcessAsset(Inputs const& inputs, std::string const& fileName);
std::string GetAssetName(std::string const& fileName);

int main(int const argc, char* argv[])
{
    try
    {
        auto const config = ParseArguments(argc, argv);
        ProcessAssets(config);
    }
    catch (std::runtime_error const& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void CheckFile(std::ios const& f, std::string const& description)
{
    if (!f)
    {
        throw std::runtime_error("Error: failed to open " + description + " file");
    }
}

Inputs ParseArguments(int const argc, char* argv[])
{
    if (argc < 5)
    {
        throw std::runtime_error("Usage: taxme year tax/dry DataLabels.cfg asset1.csv asset2.csv ...");
    }

    Inputs inputs;
    inputs.year = ParseYear(argv[1], "first argument");
    inputs.commitTax = ParseCommitTax(argv[2]);
    LoadLabels(argv[3], inputs.labelsConfig);

    for (auto i = 4; i < argc; ++i)
    {
        inputs.assetFileNames.push_back(argv[i]);
    }

    return inputs;
}

bool ParseCommitTax(std::string const& input)
{
    if (input == "tax")
    {
        return true;
    }
    else if (input == "dry")
    {
        return false;
    }
    else
    {
        throw std::runtime_error("Error: failed to parse whether to commit tax or not");
    }
}

void LoadLabels(std::string fileName, LabelsConfig& config)
{
    std::ifstream f(fileName);
    CheckFile(f, "data labels");

    std::string line;

    while (std::getline(f, line))
    {
        if (!line.empty())
        {
            LoadLabel(line, config);
        }
    }

    CheckAllLabelsDefined(config);
}

void LoadLabel(std::string const& line, LabelsConfig& config)
{
    auto const eqPos = line.find_first_of('=');

    if (eqPos == std::string::npos)
    {
        throw std::runtime_error("Error: bad format of data labels file");
    }

    auto const label = StringToLabel(line.substr(0, eqPos));
    auto const labelValue = line.substr(eqPos + 1);

    config.label2Value[label] = labelValue;
    config.value2Label[labelValue] = label;
}

void CheckAllLabelsDefined(LabelsConfig const& config)
{
    auto const requiredLabels = {
        Label::Date, Label::Type, Label::UnitPrice, Label::Quantity,
        Label::ClosedPositionQuantity, Label::Gain, Label::IsTaxed,
        Label::TypeBuy, Label::TypeSell, Label::Yes, Label::No, Label::NotAvailable};

    for (auto const label : requiredLabels)
    {
        if (config.label2Value.find(label) == config.label2Value.end())
        {
            throw std::runtime_error("Error: data labels file is missing some label");
        }
    }
}

void ProcessAssets(Inputs const& inputs)
{
    FinancialOutcome outcomeTotal = {0, 0};

    for (auto const& fileName : inputs.assetFileNames)
    {
        auto const outcome = ProcessAsset(inputs, fileName);
        outcomeTotal.income += outcome.income;
        outcomeTotal.costs += outcome.costs;
    }

    std::cout << "Total gains for year " << inputs.year << ": " << outcomeTotal << std::endl;
}

FinancialOutcome ProcessAsset(Inputs const& inputs, std::string const& fileName)
{
    std::ifstream ifile(fileName);
    CheckFile(ifile, fileName);
    Asset asset(inputs.labelsConfig, ifile);

    asset.ComputeOutputs(inputs.year, inputs.commitTax);
    auto const outcome = asset.GetFinancialOutcome(inputs.year);

    auto const assetName = GetAssetName(fileName);
    std::cout << "Gains for " << assetName << ": " << outcome << std::endl;

    auto const updatedFileName = assetName + "Updated.csv";
    std::ofstream ofile(updatedFileName);
    CheckFile(ofile, updatedFileName);
    asset.WriteUpdatedFile(inputs.labelsConfig, ofile);

    return outcome;
}

std::string GetAssetName(std::string const& fileName)
{
    static std::string csvExtension = ".csv";
    auto const extensionPos = fileName.size() - csvExtension.size();

    if (fileName.substr(extensionPos) == csvExtension)
    {
        return fileName.substr(0, extensionPos);
    }
    else
    {
        return fileName;
    }
}
