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

std::shared_ptr<Object> Object::Duplicate() const
{
    return std::make_shared<Object>(*this);
}

Object* Object::DuplicateAt(void* ptr) const
{
    return new(ptr) Object(*this);
}
