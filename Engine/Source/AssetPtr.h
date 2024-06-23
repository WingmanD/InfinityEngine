#pragma once

#include "AssetPtrBase.h"
#include "Engine/Subsystems/AssetManager.h"

template <typename T>
class AssetPtr : public AssetPtrBase
{
public:
    using type = T;

public:
    AssetPtr() : AssetPtrBase()
    {
        SetType(T::StaticType());
    }

    AssetPtr(const SharedObjectPtr<T>& asset) : AssetPtr()
    {
        SetAsset(asset);
    }

    AssetPtr(const AssetPtr& other) : AssetPtr()
    {
        SetAsset(other.GetAsset());
    }

    SharedObjectPtr<T> Get() const
    {
        return std::static_pointer_cast<T>(GetAsset());
    }

    AssetPtr& operator=(const AssetPtr& other)
    {
        SetAsset(other.GetAsset());
        return *this;
    }

    AssetPtr& operator=(const SharedObjectPtr<T>& asset)
    {
        SetAsset(asset);
        return *this;
    }

    SharedObjectPtr<T> operator->() const
    {
        return Get();
    }

    operator SharedObjectPtr<T>() const
    {
        return Get();
    }
};

template <typename T>
MemoryWriter& operator<<(MemoryWriter& writer, const AssetPtr<T>& assetPtr)
{
    const SharedObjectPtr<Asset> asset = assetPtr.GetAsset();
    if (asset == nullptr)
    {
        writer << 0ull;
    }
    else
    {
        writer << asset->GetAssetID();
    }

    return writer;
}

template <typename T>
MemoryReader& operator>>(MemoryReader& reader, AssetPtr<T>& assetPtr)
{
    uint64 assetID = 0ull;
    reader >> assetID;

    if (assetID == 0ull)
    {
        return reader;
    }

    const SharedObjectPtr<Asset> asset = AssetManager::Get().FindAsset(assetID);
    if (asset == nullptr)
    {
        return reader;
    }

    assetPtr.SetAsset(asset);

    return reader;
}
