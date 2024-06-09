#pragma once

#include "ReflectionShared.h"
#include "MulticastDelegate.h"
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
    MulticastDelegate<> OnChanged;

    enum class EEditorVisibility : uint8
    {
        None,
        Visible,
        Editable
    };

public:
    virtual ~PropertyBase() = default;

    Name GetDisplayName() const;
    const std::vector<Attribute>& GetAttributes() const;

    Type* GetType() const;

    EEditorVisibility GetEditorVisibility() const;

    virtual SharedObjectPtr<Widget> CreateWidget(const SharedObjectPtr<Object>& object);

protected:
    void SetDisplayName(Name displayName);
    std::vector<Attribute>& GetAttributes();

    void SetType(Type* type);

    void SetEditorVisibility(EEditorVisibility visibility)
    {
        _editorVisibility = visibility;
    }

private:
    Name _displayName;
    std::vector<Attribute> _attributes;
    Type* _type = nullptr;

    EEditorVisibility _editorVisibility = EEditorVisibility::None;
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
            else if (attribute.Name == "Visible")
            {
                SetEditorVisibility(EEditorVisibility::Visible);
            }
            else if (attribute.Name == "Edit")
            {
                SetEditorVisibility(EEditorVisibility::Editable);
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

    SharedObjectPtr<Widget> CreateWidget(const SharedObjectPtr<Object>& object) override
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

            if (GetEditorVisibility() == EEditorVisibility::Editable)
            {
                return ReflectionWidgets::CreateEditableWidgetForEnum(object,
                                                                      enumType, *this,
                                                                      reinterpret_cast<uint32*>(&GetRef(
                                                                          static_cast<ObjectType*>(object.get()))));
            }

            if (GetEditorVisibility() == EEditorVisibility::Visible)
            {
                return ReflectionWidgets::CreateWidgetForEnum(object,
                                                              enumType,
                                                              *this,
                                                              reinterpret_cast<uint32*>(&GetRef(
                                                                  static_cast<ObjectType*>(object.get()))));
            }
        }
        else
        {
            if (GetEditorVisibility() == EEditorVisibility::Editable)
            {
                if constexpr (ReflectionWidgets::HasEditableWidgetRepresentation<ValueType>)
                {
                    return ReflectionWidgets::CreateEditableWidgetFor(object,
                                                                      *this,
                                                                      &GetRef(static_cast<ObjectType*>(object.get())));
                }
                else
                {
                    LOG(
                        L"Could not create editable widget for property. Type {} does not have editable widget representation. See ReflectionWidgets.h for more info.",
                        Util::ToWString(NameOf<ValueType>()));
                }
            }

            if (GetEditorVisibility() == EEditorVisibility::Visible)
            {
                if constexpr (ReflectionWidgets::HasWidgetRepresentation<ValueType>)
                {
                    return ReflectionWidgets::CreateWidgetFor(object,
                                                              *this,
                                                              &GetRef(static_cast<ObjectType*>(object.get())));
                }
                else
                {
                    LOG(
                        L"Could not create widget for property. Type {} does not have widget representation. See ReflectionWidgets.h for more info.",
                        Util::ToWString(NameOf<ValueType>()));
                }
            }
        }

        return nullptr;
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
