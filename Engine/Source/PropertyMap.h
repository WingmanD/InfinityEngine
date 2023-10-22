#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

class Object;
struct PropertyBase
{
};

template <typename ObjectType, typename ValueType>
struct Property : public PropertyBase
{
public:
    Property(ValueType ObjectType::* memberPointer) : _memberPointer(memberPointer)
    {
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

private:
    ValueType ObjectType::* _memberPointer;
};

class PropertyMap
{
public:
    PropertyMap() = default;

    template <typename ObjectType, typename ValueType>
    PropertyMap& WithProperty(const std::string& name, ValueType ObjectType::* memberPointer, const std::vector<std::string> attributes = {})
    {
        GetPropertyMap()[name] = std::make_unique<Property<ObjectType, ValueType>>(memberPointer);
        PropertyBase* property = GetPropertyMap()[name].get();

        for (const std::string& attribute : attributes)
        {
            _attributeToPropertyMap[attribute].push_back(property);
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

    void ForEachPropertyWithTag(const std::string& tag, const std::function<void(PropertyBase*)>& callback) const
    {
        if (!_attributeToPropertyMap.contains(tag))
        {
            return;
        }

        for (PropertyBase* property : _attributeToPropertyMap.at(tag))
        {
            callback(property);
        }
    }

protected:
    std::unordered_map<std::string, std::unique_ptr<PropertyBase>>& GetPropertyMap()
    {
        return _propertyMap;
    }

private:
    std::unordered_map<std::string, std::unique_ptr<PropertyBase>> _propertyMap;
    std::unordered_map<std::string, std::vector<PropertyBase*>> _attributeToPropertyMap;
};
