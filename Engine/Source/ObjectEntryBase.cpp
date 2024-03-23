#include "ObjectEntryBase.h"
#include "Object.h"

ObjectEntryBase::ObjectEntryBase()
{
    ObjectType = Object::StaticType();
}

void ObjectEntryBase::SetType(Type* type)
{
    ObjectType = type;

    Object = type->NewObject();
}

Type* ObjectEntryBase::GetType() const
{
    return ObjectType;
}

SharedObjectPtr<Object> ObjectEntryBase::GetReferencedObject()
{
    return Object;
}
