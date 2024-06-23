#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "SubtypeOf.h"

class Type;

class ObjectEntryBase
{
public:
    ObjectEntryBase();
    
    void SetType(Type* type);
    Type* GetType() const;

    SharedObjectPtr<Object> GetReferencedObject();

protected:
    SubtypeOf<Object> ObjectType;
    SharedObjectPtr<Object> Object;
};
