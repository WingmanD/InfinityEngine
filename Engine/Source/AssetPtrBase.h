#pragma once

#include <memory>

class Type;
class Asset;

class AssetPtrBase
{
public:
    AssetPtrBase() = default;
    AssetPtrBase(const std::shared_ptr<Asset>& asset);

    AssetPtrBase(const AssetPtrBase& other);
    AssetPtrBase& operator=(const AssetPtrBase& other);

    bool operator==(const std::shared_ptr<Asset>& other) const;
    bool operator==(const AssetPtrBase& other) const;
    bool operator!=(const AssetPtrBase& other) const;

    void SetAsset(const std::shared_ptr<Asset>& asset);
    std::shared_ptr<Asset> GetAsset() const;

    Type* GetType() const;

    operator std::shared_ptr<Asset>() const;

protected:
    void SetType(Type* type);

private:
    std::shared_ptr<Asset> _asset;
    Type* _type = nullptr;
};
