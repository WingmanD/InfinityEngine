#include "AssetPtrBase.h"
#include "Asset.h"

AssetPtrBase::AssetPtrBase(const AssetPtrBase& other)
{
    _asset = other._asset;
    _type = other._type;
}

AssetPtrBase::AssetPtrBase(const std::shared_ptr<Asset>& asset)
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

bool AssetPtrBase::operator==(const std::shared_ptr<Asset>& other) const
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

void AssetPtrBase::SetAsset(const std::shared_ptr<Asset>& asset)
{
    _asset = asset;

    if (_asset != nullptr)
    {
        _asset->Load();
    }
}

std::shared_ptr<Asset> AssetPtrBase::GetAsset() const
{
    return _asset;
}

Type* AssetPtrBase::GetType() const
{
    return _type;
}

AssetPtrBase::operator std::shared_ptr<Asset>() const
{
    return _asset;
}

void AssetPtrBase::SetType(Type* type)
{
    _type = type;
}
