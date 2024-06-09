#include "Object.h"

Type* Object::StaticType()
{
    static Type* staticType = TypeRegistry::Get().CreateRootType<Object>();
    return staticType;
}

Type* Object::GetType() const
{
    return StaticType();
}

SharedObjectPtr<Object> Object::Duplicate() const
{
    SharedObjectPtr<Object> newObject = SharedObjectPtr<Object>(ClassBucketArray.Add(*this), ObjectDeleter<Object>());
    newObject->Copy(*this);

    return newObject;
}

Object* Object::DuplicateAt(void* ptr) const
{
    return new(ptr) Object(*this);
}

void Object::Copy(const Object& other)
{
    *this = other;
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

void Object::SetValidImplementation(bool value)
{
    _isValid = value;
}

bool Object::IsValidImplementation() const
{
    return _isValid;
}
