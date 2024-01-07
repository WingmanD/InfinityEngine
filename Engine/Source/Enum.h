#pragma once

#include "PassKey.h"
#include "ReflectionShared.h"
#include <string>
#include <unordered_map>

class EnumRegistry;

class Enum
{
public:
    Enum() = default;

    template <typename T> requires std::is_enum_v<T>
    static Enum Create(std::string name, const std::initializer_list<Attribute>& attributes, const std::initializer_list<std::pair<std::string, T>>& entries, PassKey<EnumRegistry>)
    {
        Enum newEnum(std::move(name));
        for (const auto& [entryName, value] : entries)
        {
            newEnum.AddEntry(std::move(entryName), value);
        }

        newEnum._attributes.reserve(attributes.size());
        for (const Attribute& attribute : attributes)
        {
            newEnum._attributes.push_back(attribute);

            if (!newEnum.IsBitField() && attribute.Name == "BitField")
            {
                newEnum._isBitField = true;
            }
        }

        return newEnum;
    }

    const std::string& GetName() const;

    const std::string& GetEntryName(uint32_t value) const;

    template <typename T> requires std::is_enum_v<T>
    const std::string& GetEntryName(T value) const
    {
        return GetEntryName(static_cast<uint32_t>(value));
    }

    uint32_t GetEntryValue(const std::string& name) const;

    template <typename T> requires std::is_enum_v<T>
    T GetEntryValue(const std::string& name) const
    {
        return static_cast<T>(GetEntryValue(name));
    }

    bool IsBitField() const;

    const std::vector<Attribute>& GetAttributes() const;

    const std::unordered_map<std::string, uint32_t>& GetEntries() const;

private:
    std::string _name;
    std::unordered_map<std::string, uint32_t> _entriesByName;
    std::unordered_map<uint32_t, std::string> _entriesByValue;

    bool _isBitField = false;

    std::vector<Attribute> _attributes;

private:
    Enum(std::string name);

    template <typename T>
    Enum* AddEntry(const std::string& name, T value)
    {
        const uint32_t valueAsInt = static_cast<uint32_t>(value);
        _entriesByName[name] = valueAsInt;
        _entriesByValue[valueAsInt] = name;
        
        return this;
    }
};

template <typename Enum>
struct EnableBitMaskOperators
{
    static constexpr bool Enable = false;
};

#define ENABLE_ENUM_OPS(x)  \
template<>                           \
struct EnableBitMaskOperators<x> {   \
    static const bool Enable = true; \
};

template <typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::Enable, Enum> operator |(Enum lhs, Enum rhs)
{
    using underlyingType = std::underlying_type_t<Enum>;
    return static_cast<Enum>(
        static_cast<underlyingType>(lhs) |
        static_cast<underlyingType>(rhs)
    );
}

template <typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::Enable, Enum>& operator |=(Enum& lhs, Enum rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

template <typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::Enable, Enum> operator &(Enum lhs, Enum rhs)
{
    using underlying = std::underlying_type_t<Enum>;
    return static_cast<Enum>(
        static_cast<underlying>(lhs) &
        static_cast<underlying>(rhs)
    );
}

template <typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::Enable, Enum>& operator &=(Enum& lhs, Enum rhs)
{
    using underlying = std::underlying_type_t<Enum>;
    lhs = static_cast<Enum>(
        static_cast<underlying>(lhs) &
        static_cast<underlying>(rhs)
    );
    return lhs;
}

template <typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::Enable, Enum>& operator ^=(Enum& lhs, Enum rhs)
{
    using underlying = std::underlying_type_t<Enum>;
    lhs = static_cast<Enum>(
        static_cast<underlying>(lhs) ^
        static_cast<underlying>(rhs)
    );
    return lhs;
}

template <typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::Enable, Enum> operator ^(Enum lhs, Enum rhs)
{
    using underlying = std::underlying_type_t<Enum>;
    return static_cast<Enum>(
        static_cast<underlying>(lhs) ^
        static_cast<underlying>(rhs)
    );
}

template <typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::Enable, Enum> operator ~(Enum rhs)
{
    using underlying = std::underlying_type_t<Enum>;
    return static_cast<Enum>(
        ~static_cast<underlying>(rhs)
    );
}
