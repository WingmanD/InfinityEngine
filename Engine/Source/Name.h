#pragma once

#include "CoreMinimal.h"
#include <string>

class MemoryWriter;
class MemoryReader;

class Name
{
public:
    explicit constexpr Name() = default;
    explicit Name(const std::wstring& name);    // todo this must be constexpr - register name somewhere else, write a compile time hasher
    explicit constexpr Name(uint64 id);

    uint64 GetID() const;
    const std::wstring& ToString() const;

    constexpr std::strong_ordering operator<=>(const Name& other) const;
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
