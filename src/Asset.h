#pragma once
#include "common.h"

using CSVLine = std::vector<std::string>;

struct InputData
{
    int year;
    Label type; // TypeBuy, TypeSell only
    double unitPrice;
    double quantity;
    bool isTaxed; // valid for sells only
};

struct OutputData
{
    double closedPositionQuantity; // valid for buys only
    double gain; // valid for sells only
    bool isTaxed; // valid for sells only
};

struct Record
{
    CSVLine row; // unparsed data most of which will be written back to output without modification
    InputData inputs; // selected parsed data for computing outputs
    OutputData outputs; // computed output data to be written to output
};

class Asset
{
public:

    explicit Asset(LabelsConfig const& config, std::ifstream& file);

    void ComputeOutputs(int yearReportingTaxFor, bool commitTax);
    FinancialOutcome GetFinancialOutcome(int year) const;
    void WriteUpdatedFile(LabelsConfig const& config, std::ofstream& file) const;

private:

    void LoadCSVHeader(LabelsConfig const& config, std::ifstream& file);
    void ParseHeader(LabelsConfig const& config);
    void LoadCSVData(LabelsConfig const& config, std::ifstream& file);
    void ParseInputs(LabelsConfig const& config, Record& r) const;
    CSVLine GetUpdatedCSVLine(Record const& r, LabelsConfig const& config) const;
 
    CSVLine header;
    std::map<Label, int> label2Index; // column index in CSV row    
    std::vector<Record> records;
};