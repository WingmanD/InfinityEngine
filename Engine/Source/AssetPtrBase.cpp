#include "AssetPtrBase.h"
#include "Asset.h"

AssetPtrBase::AssetPtrBase(const AssetPtrBase& other)
{
    _asset = other._asset;
    _type = other._type;
}

AssetPtrBase::AssetPtrBase(const SharedObjectPtr<Asset>& asset)
{
    _asset = asset;
}

AssetPtrBase& AssetPtrBase::operator=(const AssetPtrBase& other)
{
    if (this == &other)
    {
        return *this;
    }

    _asset = other._asset;
    _type = other._type;

    return *this;
}

bool AssetPtrBase::operator==(const SharedObjectPtr<Asset>& other) const
{
    return _asset == other;
}

bool AssetPtrBase::operator==(const AssetPtrBase& other) const
{
    return _asset == other._asset;
}

bool AssetPtrBase::operator!=(const AssetPtrBase& other) const
{
    return !(*this == other);
}

void AssetPtrBase::SetAsset(const SharedObjectPtr<Asset>& asset)
{
    _asset = asset;

    if (_asset != nullptr)
    {
        _asset->Load();
    }
}

SharedObjectPtr<Asset> AssetPtrBase::GetAsset() const
{
    return _asset;
}

Type* AssetPtrBase::GetType() const
{
    return _type;
}

AssetPtrBase::operator SharedObjectPtr<Asset>() const
{
    return _asset;
}

void AssetPtrBase::SetType(Type* type)
{
    _type = type;
}
