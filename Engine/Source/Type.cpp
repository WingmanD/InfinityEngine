#include <sstream>
#include "Type.h"
#include "Object.h"

std::shared_ptr<Object> Type::NewObject() const
{
    return GetCDO()->Duplicate();
}

void Type::AddCompositionType(Type* type)
{
    if (type == nullptr)
    {
        // todo log and debug break
        return;
    }

    // todo this should be private!
    // when composition types are added, a new type should be created
    _compositionTypes.push_back(type);

    UpdateFullID();
    UpdateFullName();
}

const Object* Type::GetCDO() const
{
    return _cdo.get();
}

uint64_t Type::HashTypeName(const std::string& str)
{
    uint64_t hash = 5381;

    for (char c : str)
    {
        hash = ((hash << 5) + hash) ^ c;
    }

    return hash;
}

void Type::ForEachPropertyWithTag(const std::string& tag, const std::function<void(PropertyBase*)>& callback) const
{
    _propertyMap.ForEachPropertyWithTag(tag, callback);

    for (const Type* parentType : _parentTypes)
    {
        parentType->ForEachPropertyWithTag(tag, callback);
    } 
}

void Type::AddParentType(Type* type)
{
    if (type == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    _parentTypes.push_back(type);
    type->_childTypes.push_back(this);
}

void Type::UpdateFullID()
{
    _fullID = _familyID;

    for (Type* compositionType : _compositionTypes)
    {
        _fullID ^= compositionType->GetID();
    }
}

void Type::UpdateFullName()
{
    std::stringstream ss;
    ss << GetName();

    if (!_compositionTypes.empty())
    {
        ss << "<";
        for (const Type* compositionType : _compositionTypes)
        {
            ss << compositionType->GetName();
        }
        ss << ">";
    }

    if (!_parentTypes.empty())
    {
        ss << " : ";
        for (const Type* parentType : _parentTypes)
        {
            ss << parentType->GetName();
        }
    }

    _fullName = ss.str();
}
