#include "NameRegistry.h"

void NameRegistry::RegisterName(Name name, const std::wstring& string)
{
    _nameMap[name] = string;
}

const std::wstring& NameRegistry::ToString(Name name) const
{
    const auto it = _nameMap.find(name);
    if (it != _nameMap.end())
    {
        return it->second;
    }
    
    return _none;
}
