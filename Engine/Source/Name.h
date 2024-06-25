#pragma once

#include "CoreMinimal.h"
#include <string>
#include <format>

class MemoryWriter;
class MemoryReader;

class Name
{
public:
    explicit constexpr Name() = default;
    explicit Name(const std::wstring& name);    // todo this must be constexpr - register name somewhere else, write a compile time hasher
    explicit constexpr Name(uint64 id);
    constexpr Name(const Name& other);

    uint64 GetID() const;
    const std::wstring& ToString() const;

    constexpr std::strong_ordering operator<=>(const Name& other) const
    {
        return _id <=> other._id;
    }
    
    constexpr bool operator==(const Name&) const = default;

private:
    uint64 _id = 0ull;
};

inline Name NameNone = Name(0ull);

MemoryReader& operator>>(MemoryReader& reader, Name& name);
MemoryWriter& operator<<(MemoryWriter& writer, const Name& name);

template <>
struct std::hash<Name>
{
    std::size_t operator()(const Name& name) const noexcept
    {
        return name.GetID();
    }
};

template <>
struct std::formatter<Name, wchar_t>
{
    constexpr auto parse(std::wformat_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const Name& name, std::wformat_context& ctx) const
    {
        return std::format_to(ctx.out(), L"{}", name.ToString());
    }
};
