#pragma once

#include <memory>

class Type;
class Asset;

class AssetPtrBase
{
public:
    AssetPtrBase() = default;
    AssetPtrBase(const SharedObjectPtr<Asset>& asset);

    AssetPtrBase(const AssetPtrBase& other);
    AssetPtrBase& operator=(const AssetPtrBase& other);

    bool operator==(const SharedObjectPtr<Asset>& other) const;
    bool operator==(const AssetPtrBase& other) const;
    bool operator!=(const AssetPtrBase& other) const;

    void SetAsset(const SharedObjectPtr<Asset>& asset);
    SharedObjectPtr<Asset> GetAsset() const;

    Type* GetType() const;

    operator SharedObjectPtr<Asset>() const;

protected:
    void SetType(Type* type);

private:
    SharedObjectPtr<Asset> _asset;
    Type* _type = nullptr;
};
