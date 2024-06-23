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
    return _newObjectInBucketArrayFactory(bucketArray);
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
    for (const Type* parentType : _parentTypes)
    {
        if (!parentType->ForEachProperty(callback))
        {
            return false;
        }
    }

    if (!_propertyMap.ForEachProperty(callback))
    {
        return false;
    }

    return true;
}

bool Type::ForEachPropertyWithTag(const std::string& tag,
                                  const std::function<bool(PropertyBase*)>& callback) const
{
    for (const Type* parentType : _parentTypes)
    {
        if (!parentType->ForEachPropertyWithTag(tag, callback))
        {
            return false;
        }
    }

    if (!_propertyMap.ForEachPropertyWithTag(tag, callback))
    {
        return false;
    }
    
    return true;
}

SharedObjectPtr<Widget> Type::CreatePropertiesWidget(const SharedObjectPtr<Object>& object) const
{
    if (object == nullptr)
    {
        DEBUG_BREAK();
        return nullptr;
    }

    SharedObjectPtr<TableWidget> table = ::NewObject<TableWidget>();
    if (!table->Initialize())
    {
        return nullptr;
    }

    ForEachProperty([&table, &object](PropertyBase* prop)
    {
        if (prop->GetEditorVisibility() == PropertyBase::EEditorVisibility::None)
        {
            return true;
        }
        
        const SharedObjectPtr<TableRowWidget> row = ::NewObject<TableRowWidget>();
        if (!row->Initialize())
        {
            return false;
        }
        table->AddRow(row);

        const SharedObjectPtr<TextBox> nameLabel = ::NewObject<TextBox>();
        if (!nameLabel->Initialize())
        {
            return false;
        }
        row->AddChild(nameLabel);

        nameLabel->SetText(prop->GetDisplayName().ToString() + L":");

        const Type* propertyType = prop->GetType();

        SharedObjectPtr<Widget> newWidget = nullptr;

        if (propertyType != nullptr && !propertyType->IsA<Asset>())
        {
            newWidget = propertyType->CreatePropertiesWidget(object);
        }
        else
        {
            newWidget = prop->CreateWidget(object);
        }

        if (newWidget == nullptr)
        {
            return false;
        }

        row->AddChild(newWidget);

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
