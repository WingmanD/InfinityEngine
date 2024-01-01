#include "EnumRegistry.h"

void EnumRegistry::Initialize(int32_t enumCount)
{
    _enums.reserve(enumCount);
}

const Enum* EnumRegistry::FindEnumByName(const std::string& name) const
{
    const auto it = _enumByName.find(name);
    if (it != _enumByName.end())
    {
        return it->second;
    }

    return nullptr;
}

const Enum* EnumRegistry::FindEnumByID(uint64_t id) const
{
    const auto it = _enumByID.find(id);
    if (it != _enumByID.end())
    {
        return it->second;
    }

    return nullptr;
}
