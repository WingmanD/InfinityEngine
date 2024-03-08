#pragma once

#include "CoreMinimal.h"
#include "Type.h"
#include <cassert>
#include <memory>
#include <unordered_map>

class BucketArrayBase;

class ObjectTypeMap
{
public:
    explicit ObjectTypeMap() = default;

    SharedObjectPtr<Object> NewObject(Type& type);

    template <typename T> requires IsA<T, Object>
    SharedObjectPtr<T> NewObject(Type& type)
    {
        assert(type.IsA<T>());

        return std::static_pointer_cast<T>(NewObject(type));
    }

    BucketArrayBase* GetBucketArrayForType(Type& type);
    BucketArrayBase& GetOrCreateBucketArrayForType(Type& type);

    /**
     * Clears all bucket arrays.
     * NOTE: Currently this invalidates shared pointers to objects stored in bucket arrays,
     * but shared pointers have no way of knowing that the object they point to has been deleted.
     */
    void Clear();

private:
    std::unordered_map<Type*, std::unique_ptr<BucketArrayBase>> _componentTypeToBucketArrayMap;
};
