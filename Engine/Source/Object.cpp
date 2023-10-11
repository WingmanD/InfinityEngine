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
