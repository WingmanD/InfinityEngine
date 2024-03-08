#pragma once

#include "CoreMinimal.h"
#include "PropertyMap.h"
#include "ReflectionTags.h"
#include "ReflectionShared.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>

class BucketArrayBase;
class Object;

class Type
{
public:
    Type* WithPropertyMap(PropertyMap propertyMap);

    Type* WithDataOffset(size_t offset);

    template <typename T>
    static uint64 CalculatePrimaryID()
    {
        uint64 id = 5381;

        id ^= HashTypeName(NameOf<T>());

        return id;
    }

    template <typename T, typename... ParentTypes>
    static uint64 CalculateFamilyID()
    {
        uint64 id = CalculatePrimaryID<T>();

        if constexpr (sizeof...(ParentTypes) > 0)
        {
            ((id ^= CalculateFamilyID<ParentTypes>()), ...);
        }

        return id;
    }

    std::shared_ptr<Object> NewObject() const;

    template <typename T>
    std::shared_ptr<T> NewObject() const
    {
        std::shared_ptr<Object> newObject = NewObject();
        return std::dynamic_pointer_cast<T>(newObject);
    }

    SharedObjectPtr<Object> NewObject(BucketArrayBase& bucketArray) const;

    template <typename T>
    SharedObjectPtr<T> NewObject(BucketArrayBase& bucketArray) const
    {
        SharedObjectPtr<Object> newObject = NewObject(bucketArray);
        return std::dynamic_pointer_cast<T>(newObject);
    }

    /*
     * Create a new object at the specified memory location.
     * NOTE: You must allocate memory for the object yourself and keep track of its lifetime.
     */
    Object* NewObjectAt(void* ptr) const;

    /*
     * Create a new object at the specified memory location.
     * NOTE: You must allocate memory for the object yourself and keep track of its lifetime.
     */
    template <typename T>
    T* NewObjectAt(void* ptr) const
    {
        Object* newObject = NewObjectAt(ptr);
        return dynamic_cast<T*>(newObject);
    }

    std::unique_ptr<BucketArrayBase> NewBucketArray() const;

    bool IsA(const Type* type) const;

    template <typename T> requires IsReflectedType<T>
    bool IsA() const
    {
        return IsA(T::StaticType());
    }

    [[nodiscard]] const Object* GetCDO() const;

    template <typename T>
    [[nodiscard]] const T* GetCDO() const
    {
        return dynamic_cast<const T*>(_cdo.get());
    }

    [[nodiscard]] uint64 GetID() const;
    [[nodiscard]] uint64 GetFamilyID() const;
    [[nodiscard]] uint64 GetFullID() const;
    [[nodiscard]] const std::string& GetName() const;
    [[nodiscard]] const std::string& GetFullName() const;

    size_t GetSize() const;
    size_t GetAlignment() const;
    size_t GetDataOffset() const;

    const std::vector<Type*>& GetParentTypes() const;
    const std::vector<Type*>& GetSubtypes() const;

    bool ForEachSubtype(const std::function<bool(Type*)>& callback, bool recursive = false);

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
            for (Type* parentType : _parentTypes)
            {
                if (auto value = parentType->GetPropertyRef<ValueType>(object, name))
                {
                    return value;
                }
            }

            return std::nullopt;
        }

        return property->GetRef(object);
    }

    bool ForEachProperty(const std::function<bool(PropertyBase*)>& callback) const;
    bool ForEachPropertyWithTag(const std::string& tag, const std::function<bool(PropertyBase*)>& callback) const;

    std::shared_ptr<Widget> CreatePropertiesWidget(const std::shared_ptr<Object>& object) const;

    friend auto operator<=>(const Type& lhs, const Type& rhs);

protected:
    Type() = default;

    ~Type() = default;

private:
    std::unique_ptr<const Object> _cdo = nullptr;   // todo create CDO in BucketArray instead of dynamic allocation

    size_t _size = 0;
    size_t _alignment = 0;
    size_t _dataOffset = 0;

    uint64 _id = -1;
    uint64 _familyID = -1;
    uint64 _fullID = -1;

    std::string _name;
    std::string _fullName;
    std::vector<Type*> _parentTypes;
    std::vector<Type*> _subtypes;

    PropertyMap _propertyMap;

    std::function<std::unique_ptr<BucketArrayBase>()> _bucketArrayFactory;
    std::function<Object*(BucketArrayBase&)> _newObjectInBucketArrayFactory;

    friend class TypeRegistry;

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

    [[nodiscard]] static uint64 HashTypeName(const std::string& str);

    void UpdateFullName();
};
