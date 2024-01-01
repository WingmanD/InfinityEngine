#pragma once

#include "Enum.h"
#include "Singleton.h"
#include <unordered_map>

class EnumRegistry : public Singleton<const EnumRegistry>
{
public:
    void Initialize(int32_t enumCount);

    template <typename T> requires std::is_enum_v<T>
    const Enum& NewEnum(std::string name, const std::initializer_list<Attribute>& attributes, const std::initializer_list<std::pair<std::string, T>>& entries)
    {
        _enums.push_back(Enum::Create(std::move(name), attributes, entries, {}));
        Enum& newEnum = _enums.back();
        _enumByName[newEnum.GetName()] = &_enums.back();

        constexpr std::hash<std::string> hasher;
        const uint64_t id = hasher(newEnum.GetName());
        _enumByID[id] = &_enums.back();

        return newEnum;
    }

    const Enum* FindEnumByName(const std::string& name) const;
    const Enum* FindEnumByID(uint64_t id) const;

    template <typename T> requires std::is_enum_v<T>
    const Enum* FindEnum() const
    {
        return FindEnumByName(typeid(T).name());
    }

private:
    std::vector<Enum> _enums;

    std::unordered_map<std::string, Enum*> _enumByName;
    std::unordered_map<uint64_t, Enum*> _enumByID;
};
