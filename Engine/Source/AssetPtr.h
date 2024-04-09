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

    AssetPtr(const std::shared_ptr<T>& asset) : AssetPtr()
    {
        SetAsset(asset);
    }

    AssetPtr(const AssetPtr& other) : AssetPtr()
    {
        SetAsset(other.GetAsset());
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

template <typename T>
MemoryWriter& operator<<(MemoryWriter& writer, const AssetPtr<T>& assetPtr)
{
    const std::shared_ptr<Asset> asset = assetPtr.GetAsset();
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

    const std::shared_ptr<Asset> asset = AssetManager::Get().FindAsset(assetID);
    if (asset == nullptr)
    {
        return reader;
    }

    assetPtr.SetAsset(asset);

    return reader;
}
