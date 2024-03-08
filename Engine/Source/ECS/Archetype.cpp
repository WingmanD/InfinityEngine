#include "Archetype.h"
#include "Entity.h"
#include "FNV1a.h"

Archetype::Archetype(const Entity& entity)
{
    _componentTypeList.reserve(entity._components.Count());

    FNV1a fnv;
    for (const SharedObjectPtr<Component>& component : entity._components)
    {
        Type* componentType = component->GetType();

        _componentTypeToIndexMap[componentType] = static_cast<uint16>(_componentTypeToIndexMap.size());
        _componentTypeList.push_back(componentType);

        fnv.Combine(componentType->GetID());
    }

    if (!_componentTypeList.empty())
    {
        _id = fnv.GetHash();
    }
}

Archetype::Archetype(const std::initializer_list<Type*>& componentTypes)
{
    _componentTypeList.reserve(componentTypes.size());

    FNV1a fnv;
    for (Type* componentType : componentTypes)
    {
        _componentTypeToIndexMap[componentType] = static_cast<uint16>(_componentTypeToIndexMap.size());
        _componentTypeList.push_back(componentType);

        fnv.Combine(componentType->GetID());
    }
    _id = fnv.GetHash();
}

uint16 Archetype::GetComponentIndex(const Type& componentType) const
{
    return _componentTypeToIndexMap.at(const_cast<Type*>(&componentType));
}

uint64 Archetype::GetID() const
{
    return _id;
}

const std::vector<Type*>& Archetype::GetComponentTypes() const
{
    return _componentTypeList;
}

int32 Archetype::SubsetIntersectionSize(const Archetype& rhs) const
{
    auto itA = rhs._componentTypeList.begin();
    auto itB = _componentTypeList.begin();

    int32 intersectCount = 0;
    while (itA != rhs._componentTypeList.end() && itB != _componentTypeList.end())
    {
        if (*itA == *itB)
        {
            ++itB;
            ++intersectCount;
        }

        ++itA;
    }

    return intersectCount;
}

bool Archetype::IsSubsetOf(const Archetype& rhs) const
{
    return SubsetIntersectionSize(rhs) != 0;
}

bool Archetype::IsSupersetOf(const Archetype& rhs) const
{
    return rhs.IsSubsetOf(*this); // todo check this
}

Archetype Archetype::Difference(const Archetype& rhs) const
{
    Archetype difference;
    FNV1a fnv;
    for (Type* type : _componentTypeList)
    {
        if (!rhs._componentTypeToIndexMap.contains(type))
        {
            difference._componentTypeToIndexMap[type] = static_cast<uint16>(difference._componentTypeToIndexMap.size());
            difference._componentTypeList.push_back(type);
            fnv.Combine(type->GetID());
        }
    }
    if (!_componentTypeList.empty())
    {
        difference._id = fnv.GetHash();
    }

    return difference;
}

Archetype Archetype::Union(const Archetype& rhs) const
{
    Archetype unionArchetype;

    FNV1a fnv;
    for (Type* type : _componentTypeList)
    {
        unionArchetype._componentTypeToIndexMap[type] = static_cast<uint16>(unionArchetype._componentTypeToIndexMap.
            size());
        unionArchetype._componentTypeList.push_back(type);
        fnv.Combine(type->GetID());
    }

    for (Type* type : rhs._componentTypeList)
    {
        if (!unionArchetype._componentTypeToIndexMap.contains(type))
        {
            unionArchetype._componentTypeToIndexMap[type] = static_cast<uint16>(unionArchetype._componentTypeToIndexMap.
                size());
            unionArchetype._componentTypeList.push_back(type);
            fnv.Combine(type->GetID());
        }
    }

    if (!unionArchetype._componentTypeList.empty())
    {
        unionArchetype._id = fnv.GetHash();
    }

    return unionArchetype;
}

Archetype Archetype::Intersection(const Archetype& rhs) const
{
    Archetype intersectionArchetype;
    FNV1a fnv;

    auto itA = rhs._componentTypeList.begin();
    auto itB = _componentTypeList.begin();

    while (itA != _componentTypeList.end() && itB != rhs._componentTypeList.end())
    {
        if (*itA == *itB)
        {
            ++itB;

            intersectionArchetype._componentTypeToIndexMap[*itA] = static_cast<uint16>(intersectionArchetype.
                _componentTypeToIndexMap.size());

            intersectionArchetype._componentTypeList.push_back(*itA);

            fnv.Combine((*itA)->GetID());
        }

        ++itA;
    }

    if (!intersectionArchetype._componentTypeList.empty())
    {
        intersectionArchetype._id = fnv.GetHash();
    }

    return intersectionArchetype;
}

std::strong_ordering Archetype::operator<=>(const Archetype& rhs) const
{
    return _id <=> rhs._id;
}
