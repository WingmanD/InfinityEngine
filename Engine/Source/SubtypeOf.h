#pragma once

#include "CoreMinimal.h"
#include "Type.h"

class MemoryReader;
class MemoryWriter;

class SubtypeOfBase
{
public:
    SubtypeOfBase() = default;

    SubtypeOfBase(const SubtypeOfBase& other);
    SubtypeOfBase(SubtypeOfBase&& other) noexcept;

    SubtypeOfBase(Type* type);

    ~SubtypeOfBase() = default;

    SubtypeOfBase& operator=(const SubtypeOfBase& other);
    SubtypeOfBase& operator=(SubtypeOfBase&& other) noexcept;

    SubtypeOfBase& operator=(Type* type);

    operator Type*() const;

    Type* operator->() const
    {
        return _type;
    }

    Type* GetType() const;

    friend MemoryWriter& operator<<(MemoryWriter& writer, const SubtypeOfBase& subtypeOf);
    friend MemoryReader& operator>>(MemoryReader& reader, SubtypeOfBase& subtypeOf);

protected:
    void SetType(Type* type);

private:
    Type* _type = nullptr;
};

MemoryWriter& operator<<(MemoryWriter& writer, const SubtypeOfBase& subtypeOf);
MemoryReader& operator>>(MemoryReader& reader, SubtypeOfBase& subtypeOf);

template <typename T> requires IsReflectedType<T>
class SubtypeOf : public SubtypeOfBase
{
public:
    SubtypeOf() : SubtypeOfBase(T::StaticType())
    {
    }

    SubtypeOf(Type* type)
    {
        if (type != nullptr && !type->IsA<T>())
        {
            DEBUG_BREAK()

            SetType(nullptr);
        }

        SetType(type);
    }

    SubtypeOf& operator=(Type* type)
    {
        if (type != nullptr && !type->IsA<T>())
        {
            DEBUG_BREAK()

            SetType(nullptr);

            return *this;
        }

        SetType(type);

        return *this;
    }
};

template <typename T> requires IsReflectedType<T>
MemoryWriter& operator<<(MemoryWriter& writer, const SubtypeOf<T>& subtype)
{
    writer << static_cast<const SubtypeOfBase&>(subtype);

    return writer;
}

template <typename T> requires IsReflectedType<T>
MemoryReader& operator>>(MemoryReader& reader, SubtypeOf<T>& subtype)
{
    reader >> static_cast<SubtypeOfBase&>(subtype);

    return reader;
}
