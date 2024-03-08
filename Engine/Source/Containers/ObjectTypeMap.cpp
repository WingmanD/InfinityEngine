#include "ObjectTypeMap.h"
#include "Containers/BucketArray.h"
#include "Object.h"
#include "Type.h"

SharedObjectPtr<Object> ObjectTypeMap::NewObject(Type& type)
{
    BucketArrayBase& bucketArray = GetOrCreateBucketArrayForType(type);
    return type.NewObject(bucketArray);
}

BucketArrayBase* ObjectTypeMap::GetBucketArrayForType(Type& type)
{
    const auto it = _componentTypeToBucketArrayMap.find(&type);
    if (it != _componentTypeToBucketArrayMap.end())
    {
        return it->second.get();
    }

    return nullptr;
}

BucketArrayBase& ObjectTypeMap::GetOrCreateBucketArrayForType(Type& type)
{
    if (BucketArrayBase* bucketArray = GetBucketArrayForType(type))
    {
        return *bucketArray;
    }

    auto [it, success] = _componentTypeToBucketArrayMap.try_emplace(&type, type.NewBucketArray());
    assert(success);

    return *it->second.get();
}

void ObjectTypeMap::Clear()
{
    _componentTypeToBucketArrayMap.clear();
}
