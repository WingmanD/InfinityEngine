#include <sstream>
#include "Type.h"
#include "Object.h"

Type* Type::WithPropertyMap(PropertyMap&& propertyMap)
{
    _propertyMap = std::move(propertyMap);
    return this;
}

Type* Type::WithDataOffset(size_t offset)
{
    _dataOffset = offset;
    return this;
}

std::shared_ptr<Object> Type::NewObject() const
{
    return GetCDO()->Duplicate();
}

Object* Type::NewObjectAt(void* ptr) const
{
    return GetCDO()->DuplicateAt(ptr);
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

bool Type::IsA(const Type* type) const
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

bool Type::HasA(const Type* type) const
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

const Object* Type::GetCDO() const
{
    return _cdo.get();
}

uint64 Type::GetID() const
{
    return _id;
}

uint64 Type::GetFamilyID() const
{
    return _familyID;
}

uint64 Type::GetFullID() const
{
    return _fullID;
}

const std::string& Type::GetName() const
{
    return _name;
}

const std::string& Type::GetFullName() const
{
    return _fullName;
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

size_t Type::GetSize() const
{
    return _size;
}

size_t Type::GetAlignment() const
{
    return _alignment;
}

size_t Type::GetAlignedSize() const
{
    return _alignedSize;
}

size_t Type::GetDataOffset() const
{
    return _dataOffset;
}

bool Type::ForEachPropertyWithTag(const std::string& tag, const std::function<bool(PropertyBase*)>& callback) const
{
    if (!_propertyMap.ForEachPropertyWithTag(tag, callback))
    {
        return false;
    }

    for (const Type* parentType : _parentTypes)
    {
        if (!parentType->ForEachPropertyWithTag(tag, callback))
        {
            return false;
        }
    }

    return true;
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

auto operator<=>(const Type& lhs, const Type& rhs)
{
    return lhs.GetID() <=> rhs.GetID();
}
