#include "PropertyMap.h"
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

bool PropertyBase::IsEditable() const
{
    return _isEditable;
}

std::shared_ptr<Widget> PropertyBase::CreateWidget(const std::shared_ptr<Object>& object)
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

void PropertyBase::SetIsEditable(bool isEditable)
{
    _isEditable = isEditable;
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
