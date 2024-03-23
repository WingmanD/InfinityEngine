#include "Object.h"

void ObjectDeleter::operator()(Object* object) const
{
    object->Destroy();
}

Type* Object::StaticType()
{
    static Type* staticType = TypeRegistry::Get().CreateRootType<Object>();
    return staticType;
}

Type* Object::GetType() const
{
    return StaticType();
}

std::shared_ptr<Object> Object::Duplicate() const
{
    return std::shared_ptr<Object>(ClassBucketArray.Add(*this), ObjectDeleter());
}

Object* Object::DuplicateAt(void* ptr) const
{
    return new(ptr) Object(*this);
}

bool Object::Serialize(MemoryWriter& writer) const
{
    return true;
}

bool Object::Deserialize(MemoryReader& reader)
{
    return true;
}

void Object::OnPropertyChanged(Name propertyName)
{
}

void Object::Destroy()
{
    if (!IsValid())
    {
        return;
    }

    ClassBucketArray.Remove(*this);
}

void Object::SetValidImplementation(bool value)
{
    _isValid = value;
}

bool Object::IsValidImplementation() const
{
    return _isValid;
}
