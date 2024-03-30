#pragma once

#include "CoreMinimal.h"
#include "Name.h"
#include "ReflectionShared.h"
#include "Containers/DArray.h"
#include <unordered_map>

class Entity;
class Component;
class MemoryReader;
class MemoryWriter;

class Archetype
{
public:
    struct QualifiedComponentType
    {
    public:
        Name Name;
        Type* Type;
        bool IsConst = false;

    public:
        auto operator==(const QualifiedComponentType& rhs) const
        {
            return Name == rhs.Name && Type == rhs.Type && IsConst == rhs.IsConst;
        }
    };

public:
    explicit Archetype() = default;
    explicit Archetype(const Entity& entity);
    explicit Archetype(const std::initializer_list<QualifiedComponentType>& componentTypes);

    template <typename... ComponentTypes> requires (IsReflectedType<ComponentTypes> && ...)
    static Archetype Create()
    {
        return Archetype({TypeChecker<ComponentTypes>::CheckConst()...});
    }

    void AddComponent(const Component& component);
    
    bool HasComponent(const Type& componentType) const;
    bool HasComponent(Name componentName) const;

    uint16 GetComponentIndex(const Type& componentType) const;
    uint16 GetComponentIndex(Name componentName) const;

    template <typename T> requires IsA<T, Component>
    uint16 GetComponentIndex() const
    {
        return GetComponentIndex(*T::StaticType());
    }

    uint64 GetID() const;

    const auto& GetComponentTypes() const
    {
        return _componentTypeList;
    }

    int32 SubsetIntersectionSize(const Archetype& rhs) const;
    bool IsSubsetOf(const Archetype& rhs) const;
    bool IsSupersetOf(const Archetype& rhs) const;

    Archetype Difference(const Archetype& rhs) const;
    Archetype Union(const Archetype& rhs) const;
    Archetype Intersection(const Archetype& rhs) const;

    bool CanBeExecutedInParallelWith(const Archetype& rhs) const;

    std::strong_ordering operator<=>(const Archetype& rhs) const;
    bool operator==(const Archetype& rhs) const = default;

    friend MemoryWriter& operator<<(MemoryWriter& writer, const Archetype& archetype);
    friend MemoryReader& operator>>(MemoryReader& reader, Archetype& archetype);

private:
    template <typename T>
    struct TypeChecker
    {
        static QualifiedComponentType CheckConst()
        {
            return {NameNone, T::StaticType(), IsConst<T>};
        }
    };

    uint64 _id = 0;
    std::unordered_map<Type*, uint16> _componentTypeToIndexMap{};
    std::unordered_map<Name, uint16> _componentNameToIndexMap{};
    DArray<QualifiedComponentType, 8> _componentTypeList{};
};

MemoryWriter& operator<<(MemoryWriter& writer, const Archetype::QualifiedComponentType& componentType);
MemoryReader& operator>>(MemoryReader& reader, Archetype::QualifiedComponentType& componentType);

MemoryWriter& operator<<(MemoryWriter& writer, const Archetype& archetype);
MemoryReader& operator>>(MemoryReader& reader, Archetype& archetype);