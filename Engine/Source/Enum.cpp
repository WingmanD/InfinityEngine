#include "Enum.h"

const std::string& Enum::GetName() const
{
    return _name;
}

const std::string& Enum::GetEntryName(uint32_t value) const
{
    return _entriesByValue.at(value);
}

uint32_t Enum::GetEntryValue(const std::string& name) const
{
    return _entriesByName.at(name);
}

bool Enum::IsBitField() const
{
    return _isBitField;
}

const std::vector<Attribute>& Enum::GetAttributes() const
{
    return _attributes;
}

const std::unordered_map<std::string, uint32_t>& Enum::GetEntries() const
{
    return _entriesByName;
}

Enum::Enum(std::string name) : _name(std::move(name))
{
}

