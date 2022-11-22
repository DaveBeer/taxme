#include "common.h"

Label StringToLabel(std::string const& labelName)
{
    if (labelName == "Date")
    {
        return Label::Date;
    }
    else if (labelName == "Type")
    {
        return Label::Type;
    }
    else if (labelName == "Type.Buy")
    {
        return Label::TypeBuy;
    }
    else if (labelName == "Type.Sell")
    {
        return Label::TypeSell;
    }
    else if (labelName == "UnitPrice")
    {
        return Label::UnitPrice;
    }
    else if (labelName == "Quantity")
    {
        return Label::Quantity;
    }
    else if (labelName == "ClosedPositionQuantity")
    {
        return Label::ClosedPositionQuantity;
    }
    else if (labelName == "Gain")
    {
        return Label::Gain;
    }
    else if (labelName == "IsTaxed")
    {
        return Label::IsTaxed;
    }
    else if (labelName == "Yes")
    {
        return Label::Yes;
    }
    else if (labelName == "No")
    {
        return Label::No;
    }
    else if (labelName == "NotAvailable")
    {
        return Label::NotAvailable;
    }

    throw std::runtime_error("Error: unknown label");
}

int ParseYear(std::string const& input, std::string const& context)
{
    std::istringstream iss(input);
    int year;
    iss >> year;

    if (!iss)
    {
        throw std::runtime_error("Error: failed to parse year from " + context);
    }

    return year;
}

std::ostream& operator<<(std::ostream& os, FinancialOutcome const& outcome)
{
    return os << outcome.income << " - " << outcome.costs << " = " << outcome.income - outcome.costs;
}
