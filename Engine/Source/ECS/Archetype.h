#pragma once

#include "CoreMinimal.h"
#include <unordered_map>

#include "ReflectionShared.h"

class Type;
class Entity;

class Archetype
{
public:
    explicit Archetype() = default;
    explicit Archetype(const Entity& entity);
    explicit Archetype(const std::initializer_list<Type*>& componentTypes);

    template <typename... ComponentTypes> requires (IsReflectedType<ComponentTypes> && ...)
    static Archetype Create()
    {
        return Archetype({ComponentTypes::StaticType()...});
    }

    uint16 GetComponentIndex(const Type& componentType) const;

    uint64 GetID() const;

    const std::vector<Type*>& GetComponentTypes() const;

    int32 SubsetIntersectionSize(const Archetype& rhs) const;
    bool IsSubsetOf(const Archetype& rhs) const;
    bool IsSupersetOf(const Archetype& rhs) const;

    Archetype Difference(const Archetype& rhs) const;
    Archetype Union(const Archetype& rhs) const;
    Archetype Intersection(const Archetype& rhs) const;

    std::strong_ordering operator<=>(const Archetype& rhs) const;
    bool operator==(const Archetype& rhs) const = default;

private:
    uint64 _id = 0;
    std::unordered_map<Type*, uint16> _componentTypeToIndexMap{};
    std::vector<Type*> _componentTypeList{};
    // todo SSO vector must be used here, this should be used if archetype contains a small number of components
};
