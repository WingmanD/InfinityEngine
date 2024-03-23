#include "SubtypeOf.h"
#include "MemoryWriter.h"
#include "MemoryReader.h"
#include "TypeRegistry.h"
#include "Object.h"

SubtypeOfBase::SubtypeOfBase(const SubtypeOfBase& other)
{
    _type = other._type;
}

SubtypeOfBase::SubtypeOfBase(SubtypeOfBase&& other) noexcept
{
    _type = other._type;
    other._type = nullptr;
}

SubtypeOfBase::SubtypeOfBase(Type* type) : _type(type)
{
}

SubtypeOfBase& SubtypeOfBase::operator=(const SubtypeOfBase& other)
{
    if (this == &other)
    {
        return *this;
    }

    _type = other._type;

    return *this;
}

SubtypeOfBase& SubtypeOfBase::operator=(SubtypeOfBase&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    _type = other._type;
    other._type = nullptr;

    return *this;
}

SubtypeOfBase& SubtypeOfBase::operator=(Type* type)
{
    _type = type;

    return *this;
}

SubtypeOfBase::operator Type*() const
{
    return _type;
}

Type* SubtypeOfBase::GetType() const
{
    return _type;
}

void SubtypeOfBase::SetType(Type* type)
{
    _type = type;
}

MemoryWriter& operator<<(MemoryWriter& writer, const SubtypeOfBase& subtypeOf)
{
    uint64 typeID = 0ull;
    if (subtypeOf._type != nullptr)
    {
        typeID = subtypeOf._type->GetID();
    }
    
    writer << typeID;
    
    return writer;
}

MemoryReader& operator>>(MemoryReader& reader, SubtypeOfBase& subtypeOf)
{
    uint64 typeID = 0ull;
    reader >> typeID;

    if (typeID == 0ull)
    {
        subtypeOf = nullptr;
    }
    else
    {
        subtypeOf = TypeRegistry::Get().FindTypeForID(typeID);
    }

    return reader;
}

