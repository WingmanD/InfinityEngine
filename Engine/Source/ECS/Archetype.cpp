﻿#include "Archetype.h"
#include "Entity.h"
#include "TypeRegistry.h"

// todo we should not initialize maps everywhere, we should have something like FullyInitialize() method that initializes maps,
// or even better, FullArchetype class that has maps - the size of this class is way too large for copying, but we need to copy it
// in async operations in World
// this class should be as cheap as possible because we need a cheaper version just to identify entities, not to query components

Archetype::Archetype(const Entity& entity)
{
    _componentTypeList.Reserve(entity._components.Count());

    FNV1a fnv;
    for (const SharedObjectPtr<Component>& component : entity._components)
    {
        Type* componentType = component->GetType();

        _componentTypeToIndexMap[componentType] = static_cast<uint16>(_componentTypeToIndexMap.size());
        _componentNameToIndexMap[component->GetName()] = static_cast<uint16>(_componentNameToIndexMap.size());
        _componentTypeList.Emplace(component->GetName(), componentType, false);

        fnv.Combine(componentType->GetID());
    }

    if (!_componentTypeList.IsEmpty())
    {
        _id = fnv.GetHash();
    }
}

void Archetype::AddComponent(const Component& component)
{
    Name name = component.GetName();
    Type* componentType = component.GetType();

    FNV1a fnv(_id);

    _componentTypeToIndexMap[componentType] = static_cast<uint16>(_componentTypeToIndexMap.size());
    _componentNameToIndexMap[name] = static_cast<uint16>(_componentNameToIndexMap.size());
    _componentTypeList.Emplace(name, componentType, false);

    fnv.Combine(componentType->GetID());
}

bool Archetype::HasComponent(const Type& componentType) const
{
    return _componentTypeToIndexMap.contains(const_cast<Type*>(&componentType));
}

bool Archetype::HasComponent(Name componentName) const
{
    return _componentNameToIndexMap.contains(componentName);
}

uint16 Archetype::GetComponentIndex(const Type& componentType) const
{
    return _componentTypeToIndexMap.at(const_cast<Type*>(&componentType));
}

uint16 Archetype::GetComponentIndex(Name componentName) const
{
    return _componentNameToIndexMap.at(componentName);
}

uint16 Archetype::GetComponentIndexChecked(Type& componentType) const
{
    const auto it = _componentTypeToIndexMap.find(&componentType);
    if (it == _componentTypeToIndexMap.end())
    {
        return std::numeric_limits<uint16>::max();
    }

    return it->second;
}

uint64 Archetype::GetID() const
{
    return _id;
}

bool Archetype::IsValid() const
{
    return GetID() != 0;
}

uint32 Archetype::StrictSubsetIntersectionSize(const Archetype& rhs) const
{
    auto itA = rhs._componentTypeList.begin();
    auto itB = _componentTypeList.begin();

    uint32 intersectCount = 0;
    while (itA != rhs._componentTypeList.end() && itB != _componentTypeList.end())
    {
        if (*itA->Type == *itB->Type)
        {
            ++itB;
            ++intersectCount;
        }

        ++itA;
    }

    return intersectCount;
}

uint32 Archetype::SubsetIntersectionSize(const Archetype& rhs) const
{
    uint32 intersectCount = 0;
    for (const QualifiedComponentType& componentTypeList : _componentTypeList)
    {
        if (rhs.HasComponent(*componentTypeList.Type))
        {
            ++intersectCount;
        }
    }

    return intersectCount;
}

bool Archetype::IsSubsetOf(const Archetype& rhs) const
{
    return SubsetIntersectionSize(rhs) == std::min(_componentTypeList.Count(), rhs._componentTypeList.Count());
}

bool Archetype::IsSupersetOf(const Archetype& rhs) const
{
    return SubsetIntersectionSize(rhs) == rhs._componentTypeList.Count() && _componentTypeList.Count() > rhs._componentTypeList.Count();
}

Archetype Archetype::Difference(const Archetype& rhs) const
{
    Archetype difference;
    FNV1a fnv;
    for (const QualifiedComponentType& qualifiedType : _componentTypeList)
    {
        if (!rhs._componentTypeToIndexMap.contains(qualifiedType.Type))
        {
            difference._componentTypeToIndexMap[qualifiedType.Type] = static_cast<uint16>(difference.
                _componentTypeToIndexMap.size());
            difference._componentNameToIndexMap[qualifiedType.Name] = static_cast<uint16>(difference.
                _componentNameToIndexMap.size());
            difference._componentTypeList.Add(qualifiedType);
            fnv.Combine(qualifiedType.Type->GetID());
        }
    }
    if (!difference._componentTypeList.IsEmpty())
    {
        difference._id = fnv.GetHash();
    }

    return difference;
}

Archetype Archetype::Union(const Archetype& rhs) const
{
    Archetype unionArchetype;

    FNV1a fnv;
    for (const QualifiedComponentType& qualifiedType : _componentTypeList)
    {
        unionArchetype._componentTypeToIndexMap[qualifiedType.Type] = static_cast<uint16>(unionArchetype.
            _componentTypeToIndexMap.size());
        unionArchetype._componentNameToIndexMap[qualifiedType.Name] = static_cast<uint16>(unionArchetype.
            _componentNameToIndexMap.size());
        unionArchetype._componentTypeList.Add(qualifiedType);
        fnv.Combine(qualifiedType.Type->GetID());
    }

    for (const QualifiedComponentType& qualifiedType : rhs._componentTypeList)
    {
        if (!unionArchetype._componentTypeToIndexMap.contains(qualifiedType.Type))
        {
            unionArchetype._componentTypeToIndexMap[qualifiedType.Type] = static_cast<uint16>(unionArchetype.
                _componentTypeToIndexMap.size());
            unionArchetype._componentNameToIndexMap[qualifiedType.Name] = static_cast<uint16>(unionArchetype.
                _componentNameToIndexMap.size());
            unionArchetype._componentTypeList.Add(qualifiedType);
            fnv.Combine(qualifiedType.Type->GetID());
        }
    }

    if (!unionArchetype._componentTypeList.IsEmpty())
    {
        unionArchetype._id = fnv.GetHash();
    }

    return unionArchetype;
}

Archetype Archetype::StrictIntersection(const Archetype& rhs) const
{
    Archetype intersectionArchetype;
    FNV1a fnv;

    auto itA = rhs._componentTypeList.begin();
    auto itB = _componentTypeList.begin();

    while (itA != rhs._componentTypeList.end() && itB != _componentTypeList.end())
    {
        if (*itA->Type == *itB->Type)
        {
            ++itB;

            intersectionArchetype._componentTypeToIndexMap[itA->Type] = static_cast<uint16>(intersectionArchetype.
                _componentTypeToIndexMap.size());
            intersectionArchetype._componentNameToIndexMap[itA->Name] = static_cast<uint16>(intersectionArchetype.
                _componentNameToIndexMap.size());

            intersectionArchetype._componentTypeList.Emplace(itA->Name, itA->Type, itA->IsConst && itB->IsConst);

            fnv.Combine(itA->Type->GetID());
        }

        ++itA;
    }

    if (!intersectionArchetype._componentTypeList.IsEmpty())
    {
        intersectionArchetype._id = fnv.GetHash();
    }

    return intersectionArchetype;
}

Archetype Archetype::Intersection(const Archetype& rhs) const
{
    Archetype intersectionArchetype;
    FNV1a fnv;
    
    for (const QualifiedComponentType& componentTypeList : _componentTypeList)
    {
        if (rhs.HasComponent(*componentTypeList.Type))
        {
            intersectionArchetype._componentTypeToIndexMap[componentTypeList.Type] = static_cast<uint16>(intersectionArchetype._componentTypeToIndexMap.size());
            intersectionArchetype._componentNameToIndexMap[componentTypeList.Name] = static_cast<uint16>(intersectionArchetype._componentNameToIndexMap.size());
            intersectionArchetype._componentTypeList.Add(componentTypeList);
            
            fnv.Combine(componentTypeList.Type->GetID());
        }
    } 

    if (!intersectionArchetype._componentTypeList.IsEmpty())
    {
        intersectionArchetype._id = fnv.GetHash();
    }

    return intersectionArchetype;
}

bool Archetype::CanBeExecutedInParallelWith(const Archetype& rhs) const
{
    const Archetype intersection = StrictIntersection(rhs);
    for (const QualifiedComponentType& qualifiedType : intersection._componentTypeList)
    {
        if (!qualifiedType.IsConst)
        {
            return false;
        }
    }

    return true;
}

std::strong_ordering Archetype::operator<=>(const Archetype& rhs) const
{
    return _id <=> rhs._id;
}

MemoryWriter& operator<<(MemoryWriter& writer, const Archetype::QualifiedComponentType& componentType)
{
    writer << componentType.Name;
    writer << (componentType.Type ? componentType.Type->GetID() : 0ull);
    writer << componentType.IsConst;

    return writer;
}

MemoryReader& operator>>(MemoryReader& reader, Archetype::QualifiedComponentType& componentType)
{
    reader >> componentType.Name;

    uint64 typeID = 0;
    reader >> typeID;

    reader >> componentType.IsConst;

    componentType.Type = TypeRegistry::Get().FindTypeForID(typeID);

    return reader;
}

MemoryWriter& operator<<(MemoryWriter& writer, const Archetype& archetype)
{
    writer << archetype._componentTypeList;

    return writer;
}

MemoryReader& operator>>(MemoryReader& reader, Archetype& archetype)
{
    reader >> archetype._componentTypeList;

    FNV1a fnv;
    uint16 index = 0;
    for (const Archetype::QualifiedComponentType& qualifiedType : archetype._componentTypeList)
    {
        // todo check if error is here
        archetype._componentTypeToIndexMap[qualifiedType.Type] = index;
        archetype._componentNameToIndexMap[qualifiedType.Name] = index;

        fnv.Combine(qualifiedType.Type->GetID());
        
        ++index;
    }
    archetype._id = fnv.GetHash();

    return reader;
}
