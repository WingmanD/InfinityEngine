﻿#pragma once

#include "ReflectionShared.h"
#include "Delegate.h"
#include "EnumRegistry.h"
#include "ReflectionWidgets.h"
#include "Name.h"
#include "Util.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

class Widget;
class Type;
class Object;

struct PropertyBase
{
public:
    Delegate<> OnChanged;

public:
    virtual ~PropertyBase() = default;

    Name GetDisplayName() const;
    const std::vector<Attribute>& GetAttributes() const;

    Type* GetType() const;

    bool IsEditable() const;

    virtual std::shared_ptr<Widget> CreateWidget(const std::shared_ptr<Object>& object);

protected:
    void SetDisplayName(Name displayName);
    std::vector<Attribute>& GetAttributes();

    void SetType(Type* type);

    void SetIsEditable(bool isEditable);

private:
    Name _displayName;
    std::vector<Attribute> _attributes;
    Type* _type = nullptr;

    bool _isEditable = false;
};

template <typename ObjectType, typename ValueType>
struct Property : public PropertyBase
{
public:
    Property(const std::string& name, Type* type, ValueType ObjectType::* memberPointer,
             const std::initializer_list<Attribute>& attributes) : _memberPointer(memberPointer)
    {
        SetType(type);

        std::vector<Attribute>& attrs = GetAttributes();
        for (const Attribute& attribute : attributes)
        {
            attrs.push_back(attribute);
            if (attribute.Name == "DisplayName")
            {
                SetDisplayName(Name(Util::ToWString(attribute.Value)));
            }
            else if (attribute.Name == "Edit")
            {
                SetIsEditable(true);
            }
        }

        if (GetDisplayName() == NameNone)
        {
            SetDisplayName(Name(Util::ToWString(name)));
        }
    }

    ValueType ObjectType::* GetMemberPointer() const
    {
        return _memberPointer;
    }

    ValueType Get(const ObjectType* object) const
    {
        return object->*_memberPointer;
    }

    ValueType& GetRef(ObjectType* object)
    {
        return object->*_memberPointer;
    }

    std::shared_ptr<Widget> CreateWidget(const std::shared_ptr<Object>& object) override
    {
        if constexpr (std::is_enum_v<ValueType>)
        {
            const Enum* enumType = EnumRegistry::Get().FindEnum<ValueType>();
            if (enumType == nullptr)
            {
                LOG(
                    L"Could not create widget for enum property. Enum type {} not registered. Put REFLECTED() above the enum declaration.",
                    Util::ToWString(NameOf<ValueType>()));
                return nullptr;
            }

            if (IsEditable())
            {
                return ReflectionWidgets::CreateEditableWidgetForEnum(object, enumType, *this,
                                                                      reinterpret_cast<uint32*>(&
                                                                          GetRef(
                                                                              static_cast<ObjectType*>(object.get()))));
            }

            return ReflectionWidgets::CreateWidgetForEnum(object, enumType, *this,
                                                          reinterpret_cast<uint32*>(&GetRef(
                                                              static_cast<ObjectType*>(object.get()))));
        }
        else
        {
            if (IsEditable())
            {
                return ReflectionWidgets::CreateEditableWidgetFor(object, *this,
                                                                  &GetRef(static_cast<ObjectType*>(object.get())));
            }

            return ReflectionWidgets::CreateWidgetFor(object, *this, &GetRef(static_cast<ObjectType*>(object.get())));
        }
    }

private:
    ValueType ObjectType::* _memberPointer;
};

class PropertyMap
{
public:
    PropertyMap() = default;

    template <typename ObjectType, typename ValueType>
    PropertyMap& WithProperty(const std::string& name, Type* type,
                              ValueType ObjectType::* memberPointer,
                              const std::initializer_list<Attribute>& attributes = {})
    {
        GetPropertyMap()[name] = std::make_unique<Property<ObjectType, ValueType>>(
            name, type, memberPointer, attributes);
        PropertyBase* property = GetPropertyMap()[name].get();

        for (const Attribute& attribute : attributes)
        {
            _attributeToPropertyMap[attribute.Name].push_back(property);
        }

        return *this;
    }

    template <typename ObjectType, typename PropertyType>
    Property<ObjectType, PropertyType>* GetProperty(const std::string& name) const
    {
        if (!_propertyMap.contains(name))
        {
            return nullptr;
        }

        PropertyBase* property = _propertyMap.at(name).get();
        return static_cast<Property<ObjectType, PropertyType>*>(property);
    }

    bool ForEachPropertyWithTag(const std::string& tag, const std::function<bool(PropertyBase*)>& callback) const;

    bool ForEachProperty(const std::function<bool(PropertyBase*)>& callback) const;

protected:
    std::unordered_map<std::string, std::unique_ptr<PropertyBase>>& GetPropertyMap();

private:
    std::unordered_map<std::string, std::unique_ptr<PropertyBase>> _propertyMap;
    std::unordered_map<std::string, std::vector<PropertyBase*>> _attributeToPropertyMap;
};
