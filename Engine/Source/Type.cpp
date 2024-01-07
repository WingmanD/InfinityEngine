#include <sstream>
#include "Type.h"
#include "Object.h"
#include "Rendering/Widgets/FlowBox.h"
#include "Rendering/Widgets/TextBox.h"

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

    for (const Type* parentType : _parentTypes)
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

    for (const Type* compositionType : _compositionTypes)
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

size_t Type::GetAlignedSize() const
{
    return _alignedSize;
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


    std::shared_ptr<FlowBox> propertiesWidget = std::make_shared<FlowBox>();
    if (!propertiesWidget->Initialize())
    {
        return nullptr;
    }

    propertiesWidget->SetDirection(EFlowBoxDirection::Vertical);
    propertiesWidget->SetCollisionEnabled(false);

    ForEachProperty([&propertiesWidget, &object](PropertyBase* prop)
    {
        const std::shared_ptr<FlowBox> propertyHorizontalBox = std::make_shared<FlowBox>();
        if (!propertyHorizontalBox->Initialize())
        {
            return false;
        }

        propertyHorizontalBox->SetDirection(EFlowBoxDirection::Horizontal);
        propertyHorizontalBox->SetCollisionEnabled(false);
        propertiesWidget->AddChild(propertyHorizontalBox);

        const std::shared_ptr<TextBox> nameLabel = std::make_shared<TextBox>();
        if (!nameLabel->Initialize())
        {
            return false;
        }
        propertyHorizontalBox->AddChild(nameLabel);

        nameLabel->SetText(prop->GetDisplayName() + L":");

        const Type* propertyType = prop->GetType();
        if (propertyType != nullptr && !propertyType->IsA<Asset>())
        {
            if (const std::shared_ptr<Widget> newWidget = propertyType->CreatePropertiesWidget(object))
            {
                propertyHorizontalBox->AddChild(newWidget);
            }
        }
        else
        {
            if (const std::shared_ptr<Widget> newWidget = prop->CreateWidget(object))
            {
                propertyHorizontalBox->AddChild(newWidget);
            }
        }

        return true;
    });

    return propertiesWidget;
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

void Type::UpdateFullID()
{
    _fullID = _familyID;

    for (const Type* compositionType : _compositionTypes)
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
