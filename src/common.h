#pragma once
#include <utility>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>

struct FinancialOutcome
{
    double income;
    double costs;
};

enum class Label
{
    // input columns
    Date,
    Type,
    UnitPrice,
    Quantity,

    // output columns
    ClosedPositionQuantity,
    Gain,
    IsTaxed, // also input column (keep Yes values)

    // values
    TypeBuy,
    TypeSell,
    Yes,
    No,
    NotAvailable
};

struct LabelsConfig
{
    std::map<Label, std::string> label2Value;
    std::map<std::string, Label> value2Label;
};

Label StringToLabel(std::string const& labelName);

int ParseYear(std::string const& input, std::string const& context);

std::ostream& operator<<(std::ostream& os, FinancialOutcome const& outcome);
