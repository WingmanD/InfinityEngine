#pragma once

#include "AssetPtrBase.h"
#include "Asset.h"

template <typename T>
class AssetPtr : public AssetPtrBase
{
public:
    using type = T;
    
public:
    AssetPtr()
    {
        SetType(T::StaticType());
    }

    AssetPtr(const std::shared_ptr<T>& asset) : AssetPtrBase(asset)
    {
        SetType(T::StaticType());
    }

    AssetPtr(const AssetPtr& other) : AssetPtrBase(other)
    {
        SetType(T::StaticType());
    }

    AssetPtr& operator=(const AssetPtr& other)
    {
        SetAsset(other.GetAsset());
        return *this;
    }

    AssetPtr& operator=(const std::shared_ptr<T>& asset)
    {
        SetAsset(asset);
        return *this;
    }

    std::shared_ptr<T> operator->() const
    {
        return std::static_pointer_cast<T>(GetAsset());
    }

    operator std::shared_ptr<T>() const
    {
        return std::static_pointer_cast<T>(GetAsset());
    }
};
