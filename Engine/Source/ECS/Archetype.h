#pragma once

#include "CoreMinimal.h"
#include "FNV1a.h"
#include "Name.h"
#include "ReflectionShared.h"
#include "Type.h"
#include "TypeSet.h"
#include "Containers/DArray.h"
#include <format>
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

    template <typename Container>
    static Archetype CreateFrom(const Container& componentTypes)
    {
        Archetype archetype;

        size_t count;
        if constexpr (IsIEContainer<Container>)
        {
            count = componentTypes.Count();
        }
        else
        {
            count = componentTypes.size();
        }
        
        archetype._componentTypeList.Reserve(count);

        FNV1a fnv;
        for (const QualifiedComponentType& qualifiedType : componentTypes)
        {
            archetype._componentTypeToIndexMap[qualifiedType.Type] = static_cast<uint16>(archetype._componentTypeToIndexMap.size());
            archetype._componentNameToIndexMap[qualifiedType.Name] = static_cast<uint16>(archetype._componentNameToIndexMap.size());
            archetype._componentTypeList.Emplace(qualifiedType);

            fnv.Combine(qualifiedType.Type->GetID());
        }
        archetype._id = fnv.GetHash();

        return archetype;
    }

    template <typename... ComponentTypes> requires (IsReflectedType<ComponentTypes> && ...)
    static Archetype Create()
    {
        return CreateFrom<std::initializer_list<QualifiedComponentType>>({TypeChecker<ComponentTypes>::CheckConst()...});
    }

    template <typename ComponentTypes> requires IsA<ComponentTypes, TypeSetBase>
    static Archetype Create()
    {
        DArray<QualifiedComponentType> componentTypes;
        ComponentTypes::ForEach([&]<typename T>()
        {
            componentTypes.Add(TypeChecker<T>::CheckConst());
        });

        return CreateFrom(componentTypes);
    }

    void AddComponent(const Component& component);
    
    bool HasComponent(const Type& componentType) const;
    bool HasComponent(Name componentName) const;

    template <typename T> requires IsA<T, Component>
    bool HasComponent() const
    {
        return HasComponent(*T::StaticType());
    }

    uint16 GetComponentIndex(const Type& componentType) const;
    uint16 GetComponentIndex(Name componentName) const;

    template <typename T> requires IsA<T, Component>
    uint16 GetComponentIndex() const
    {
        return GetComponentIndex(*T::StaticType());
    }

    uint16 GetComponentIndexChecked(Type& componentType) const;
    
    template <typename T> requires IsA<T, Component>
    uint16 GetComponentIndexChecked() const
    {
        return GetComponentIndexChecked(*T::StaticType());
    }

    uint64 GetID() const;
    bool IsValid() const;

    const auto& GetComponentTypes() const
    {
        return _componentTypeList;
    }

    uint32 StrictSubsetIntersectionSize(const Archetype& rhs) const;
    uint32 SubsetIntersectionSize(const Archetype& rhs) const;
    bool IsSubsetOf(const Archetype& rhs) const;
    bool IsSupersetOf(const Archetype& rhs) const;

    Archetype Difference(const Archetype& rhs) const;
    Archetype Union(const Archetype& rhs) const;
    Archetype StrictIntersection(const Archetype& rhs) const;
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

template <>
struct std::formatter<Archetype, wchar_t>
{
    constexpr auto parse(std::wformat_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const Archetype& archetype, std::wformat_context& ctx) const
    {
        auto out = ctx.out();
        
        std::format_to(out, L"Archetype<");

        const DArray<Archetype::QualifiedComponentType, 8>& types = archetype.GetComponentTypes();
        if (types.IsEmpty())
        {
            std::format_to(out, L">");
            return out;
        }
        
        for (int32 i = 0; i < types.Count() - 1; ++i)
        {
            std::format_to(out, L"{},", Util::ToWString(types[i].Type->GetName()));
        }
        std::format_to(out, L"{}>", Util::ToWString(types[types.Count() - 1].Type->GetName()));
        
        return out;
    }
};
