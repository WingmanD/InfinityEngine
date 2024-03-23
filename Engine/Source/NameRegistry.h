#pragma once

#include "Name.h"
#include "Singleton.h"
#include <unordered_map>

class NameRegistry : public Singleton<NameRegistry>
{
public:
    void RegisterName(Name name, const std::wstring& string);
    const std::wstring& ToString(Name name) const;

private:
    std::unordered_map<Name, std::wstring> _nameMap;

    std::wstring _none = L"None";
};
