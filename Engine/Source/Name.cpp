﻿#include "Name.h"
#include "NameRegistry.h"
#include "MemoryReader.h"
#include "MemoryWriter.h"

Name::Name(const std::wstring& name)
{
    _id = std::hash<std::wstring>{}(name);
    NameRegistry::Get().RegisterName(*this, name);
}

constexpr Name::Name(uint64 id)
{
    _id = id;
}

uint64 Name::GetID() const
{
    return _id;
}

const std::wstring& Name::ToString() const
{
    return NameRegistry::Get().ToString(*this);
}

constexpr std::strong_ordering Name::operator<=>(const Name& other) const
{
    return _id <=> other._id;
}

MemoryReader& operator>>(MemoryReader& reader, Name& name)
{
    std::wstring nameString;
    reader >> nameString;

    name = Name(nameString);

    return reader;
}

MemoryWriter& operator<<(MemoryWriter& writer, const Name& name)
{
    writer << name.ToString();

    return writer;
}
