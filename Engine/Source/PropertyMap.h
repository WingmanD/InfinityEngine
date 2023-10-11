#pragma once

#include <memory>
#include <string>
#include <unordered_map>

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
    PropertyMap& WithProperty(const std::string& name, ValueType ObjectType::* memberPointer)
    {
        GetPropertyMap()[name] = std::make_unique<Property<ObjectType, ValueType>>(memberPointer);
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

protected:
    std::unordered_map<std::string, std::unique_ptr<PropertyBase>>& GetPropertyMap()
    {
        return _propertyMap;
    }

private:
    std::unordered_map<std::string, std::unique_ptr<PropertyBase>> _propertyMap;
};
