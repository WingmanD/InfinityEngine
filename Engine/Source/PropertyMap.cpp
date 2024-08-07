﻿#include "PropertyMap.h"
#include "Rendering/Widgets/Widget.h"
#include <ranges>

Name PropertyBase::GetDisplayName() const
{
    return _displayName;
}

const std::vector<Attribute>& PropertyBase::GetAttributes() const
{
    return _attributes;
}

Type* PropertyBase::GetType() const
{
    return _type;
}

PropertyBase::EEditorVisibility PropertyBase::GetEditorVisibility() const
{
    return _editorVisibility;
}

SharedObjectPtr<Widget> PropertyBase::CreateWidget(const SharedObjectPtr<Object>& object)
{
    return nullptr;
}

void PropertyBase::SetDisplayName(Name displayName)
{
    _displayName = displayName;
}

std::vector<Attribute>& PropertyBase::GetAttributes()
{
    return _attributes;
}

void PropertyBase::SetType(Type* type)
{
    _type = type;
}

bool PropertyMap::ForEachPropertyWithTag(const std::string& tag,
                                         const std::function<bool(PropertyBase*)>& callback) const
{
    if (!_attributeToPropertyMap.contains(tag))
    {
        return false;
    }

    for (PropertyBase* property : _attributeToPropertyMap.at(tag))
    {
        if (!callback(property))
        {
            return false;
        }
    }

    return true;
}

bool PropertyMap::ForEachProperty(const std::function<bool(PropertyBase*)>& callback) const
{
    for (const auto& property : _propertyMap | std::views::values)
    {
        if (!callback(property.get()))
        {
            return false;
        }
    }

    return true;
}

std::unordered_map<std::string, std::unique_ptr<PropertyBase>>& PropertyMap::GetPropertyMap()
{
    return _propertyMap;
}
