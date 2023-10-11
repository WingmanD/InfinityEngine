#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <optional>
#include <typeinfo>

#include "PropertyMap.h"
#include "ReflectionTags.h"

class Object;

#define TYPE(Derived, ...) \
public: \
    static Type* StaticType() { \
        static Type* staticType = TypeRegistry::Get().CreateType<Derived, ##__VA_ARGS__>(); \
        return staticType; \
    } \
    \
    virtual Type* GetType() const override { \
        return StaticType(); \
    } \
private:

template <typename T>
concept IsReflectedType = requires
{
    T::StaticType();
};

class Type
{
public:
    Type* WithPropertyMap(PropertyMap&& propertyMap)
    {
        _propertyMap = std::move(propertyMap);
        return this;
    }

    template <typename T>
    static std::string GetTypeName()
    {
        const std::string name = typeid(T).name();
        return name.substr(name.find(' ') + 1);
    }

    template <typename T>
    static uint64_t CalculatePrimaryID()
    {
        uint64_t id = 5381;

        id ^= HashTypeName(GetTypeName<T>());

        return id;
    }

    template <typename T, typename... ParentTypes>
    static uint64_t CalculateFamilyID()
    {
        uint64_t id = CalculatePrimaryID<T>();

        if constexpr (sizeof...(ParentTypes) > 0)
        {
            ((id ^= CalculateFamilyID<ParentTypes>()), ...);
        }

        return id;
    }

    template <typename... CompositionTypes>
    void AddCompositionTypes()
    {
        //AddCompositionType(CreateType<CompositionTypes>()...);
    }

    void AddCompositionType(Type* type);

    bool IsA(Type* type) const
    {
        if (_id == type->_id)
        {
            return true;
        }

        for (Type* parentType : _parentTypes)
        {
            if (parentType->IsA(type))
            {
                return true;
            }
        }

        return false;
    }

    bool HasA(Type* type) const
    {
        if (_id == type->_id)
        {
            return true;
        }

        for (Type* compositionType : _compositionTypes)
        {
            if (compositionType->IsA(type))
            {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] const Object* GetCDO() const
    {
        return _cdo.get();
    }

    template <typename T>
    [[nodiscard]] const T* GetCDO() const
    {
        return dynamic_cast<const T*>(_cdo.get());
    }

    [[nodiscard]] uint64_t GetID() const
    {
        return _id;
    }

    [[nodiscard]] uint64_t GetFamilyID() const
    {
        return _familyID;
    }

    [[nodiscard]] uint64_t GetFullID() const
    {
        return _fullID;
    }

    [[nodiscard]] const std::string& GetName() const
    {
        return _name;
    }

    [[nodiscard]] const std::string& GetFullName() const
    {
        return _fullName;
    }

    template <typename ValueType, typename ObjectType>
    std::optional<ValueType> GetProperty(const ObjectType* object, const std::string& name) const
    {
        auto property = _propertyMap.GetProperty<ObjectType, ValueType>(name);
        if (property == nullptr)
        {
            for (Type* parentType : _parentTypes)
            {
                if (std::optional<ValueType> value = parentType->GetProperty<ValueType>(object, name))
                {
                    return value;
                }
            }

            return std::nullopt;
        }

        return property->Get(object);
    }

    template <typename ValueType, typename ObjectType>
    std::optional<std::reference_wrapper<ValueType>> GetPropertyRef(ObjectType* object, const std::string& name) const
    {
        auto property = _propertyMap.GetProperty<ObjectType, ValueType>(name);
        if (property == nullptr)
        {
            return std::nullopt;
        }

        return property->GetRef(object);
    }

protected:
    Type() = default;

    ~Type() = default;

private:
    void AddParentType(Type* type);

    template <typename T>
    void AddParentType()
    {
        if constexpr (IsReflectedType<T>)
        {
            AddParentType(T::StaticType());
        }
    }

    [[nodiscard]] static uint64_t HashTypeName(const std::string& str);

    void UpdateFullName();

    void UpdateFullID();

    std::unique_ptr<const Object> _cdo = nullptr;

    uint64_t _id = -1;
    uint64_t _familyID = -1;
    uint64_t _fullID = -1;

    std::string _name;
    std::string _fullName;
    std::vector<Type*> _parentTypes;
    std::vector<Type*> _childTypes;

    std::vector<Type*> _compositionTypes;

    PropertyMap _propertyMap;

    friend class TypeRegistry;
};
