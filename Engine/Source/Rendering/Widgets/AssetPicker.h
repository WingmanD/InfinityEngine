#pragma once

#include "DropdownTextChoice.h"
#include "DropdownMenu.h"
#include "AssetPicker.reflection.h"

REFLECTED()
class DropdownAssetChoice : public DropdownTextChoice
{
    GENERATED()

public:
    DropdownAssetChoice() = default;

    DropdownAssetChoice(const DropdownAssetChoice& other);
    DropdownAssetChoice& operator=(const DropdownAssetChoice& other);

    bool InitializeFromAsset(const std::shared_ptr<Asset>& asset);

    std::shared_ptr<Asset> GetSelectedAsset() const;

    // DropdownTextChoice
public:
    virtual bool Initialize() override;

private:
    std::weak_ptr<Asset> _asset;

private:
    bool InitializeFromAssetInternal(const std::shared_ptr<Asset>& asset);
};

REFLECTED()
class AssetPicker : public DropdownMenu
{
    GENERATED()

public:
    AssetPicker() = default;

    AssetPicker(const AssetPicker& other) = default;
    AssetPicker& operator=(const AssetPicker& other) = default;

    static std::shared_ptr<AssetPicker> CreateForType(Type* assetType);

    void InitializeFromAssetType(Type* assetType);

    Type* GetAssetType() const;

    std::shared_ptr<Asset> GetSelectedAsset() const;
    void SetSelectedAsset(const std::shared_ptr<Asset>& asset);

private:
    Type* _assetType = nullptr;
};
