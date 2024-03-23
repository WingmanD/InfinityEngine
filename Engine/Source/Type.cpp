#include "Type.h"
#include "Object.h"
#include "Rendering/Widgets/TableWidget.h"
#include "Rendering/Widgets/TextBox.h"
#include <sstream>

Type* Type::WithPropertyMap(PropertyMap propertyMap)
{
    _propertyMap = std::move(propertyMap);
    return this;
}

Type* Type::WithDataOffset(size_t offset)
{
    _dataOffset = offset;
    return this;
}

SharedObjectPtr<Object> Type::NewObject() const
{
    return GetCDO()->Duplicate();
}

Object* Type::NewObjectAt(void* ptr) const
{
    return GetCDO()->DuplicateAt(ptr);
}

SharedObjectPtr<Object> Type::NewObject(BucketArrayBase& bucketArray) const
{
    return SharedObjectPtr<Object>(_newObjectInBucketArrayFactory(bucketArray), ObjectDeleter());
}

std::unique_ptr<BucketArrayBase> Type::NewBucketArray() const
{
    return _bucketArrayFactory();
}

bool Type::IsA(const Type* type) const
{
    if (_id == type->_id)
    {
        return true;
    }

    for (const Type* parentType : _parentTypes)
    {
        if (parentType->IsA(type))
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

    for (const char c : str)
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

size_t Type::GetDataOffset() const
{
    return _dataOffset;
}

const std::vector<Type*>& Type::GetParentTypes() const
{
    return _parentTypes;
}

const std::vector<Type*>& Type::GetSubtypes() const
{
    return _subtypes;
}

bool Type::ForEachSubtype(const std::function<bool(Type*)>& callback, bool recursive)
{
    callback(this);

    if (recursive)
    {
        for (Type* subtype : _subtypes)
        {
            if (!subtype->ForEachSubtype(callback, recursive))
            {
                return false;
            }
        }
    }

    return true;
}

bool Type::ForEachProperty(const std::function<bool(PropertyBase*)>& callback) const
{
    if (!_propertyMap.ForEachProperty(callback))
    {
        return false;
    }

    for (const Type* parentType : _parentTypes)
    {
        if (!parentType->ForEachProperty(callback))
        {
            return false;
        }
    }

    return true;
}

bool Type::ForEachPropertyWithTag(const std::string& tag,
                                  const std::function<bool(PropertyBase*)>& callback) const
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

std::shared_ptr<Widget> Type::CreatePropertiesWidget(const std::shared_ptr<Object>& object) const
{
    if (object == nullptr)
    {
        DEBUG_BREAK();
        return nullptr;
    }


    std::shared_ptr<TableWidget> table = std::make_shared<TableWidget>();
    if (!table->Initialize())
    {
        return nullptr;
    }

    ForEachProperty([&table, &object](PropertyBase* prop)
    {
        const std::shared_ptr<TableRowWidget> row = std::make_shared<TableRowWidget>();
        if (!row->Initialize())
        {
            return false;
        }
        table->AddRow(row);

        const std::shared_ptr<TextBox> nameLabel = std::make_shared<TextBox>();
        if (!nameLabel->Initialize())
        {
            return false;
        }
        row->AddChild(nameLabel);

        nameLabel->SetText(prop->GetDisplayName().ToString() + L":");

        const Type* propertyType = prop->GetType();
        if (propertyType != nullptr && !propertyType->IsA<Asset>())
        {
            if (const std::shared_ptr<Widget> newWidget = propertyType->CreatePropertiesWidget(object))
            {
                row->AddChild(newWidget);
            }
        }
        else
        {
            if (const std::shared_ptr<Widget> newWidget = prop->CreateWidget(object))
            {
                row->AddChild(newWidget);
            }
        }

        return true;
    });

    return table;
}

bool Type::operator==(const Type& rhs) const
{
    return GetID() == rhs.GetID();
}

void Type::AddParentType(Type* type)
{
    if (type == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    _parentTypes.push_back(type);
    type->_subtypes.push_back(this);
}

void Type::UpdateFullName()
{
    std::stringstream ss;
    ss << GetName();

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
