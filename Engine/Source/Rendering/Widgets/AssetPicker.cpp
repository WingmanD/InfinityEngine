#include "AssetPicker.h"
#include "TextBox.h"
#include "Engine/Subsystems/AssetManager.h"

DropdownAssetChoice::DropdownAssetChoice(const DropdownAssetChoice& other) : DropdownTextChoice(other)
{
    _asset = other._asset;
}

DropdownAssetChoice& DropdownAssetChoice::operator=(const DropdownAssetChoice& other)
{
    if (this == &other)
    {
        return *this;
    }

    return *this;
}

bool DropdownAssetChoice::InitializeFromAsset(const SharedObjectPtr<Asset>& asset)
{
    if (!Initialize())
    {
        return false;
    }

    return InitializeFromAssetInternal(asset);
}

SharedObjectPtr<Asset> DropdownAssetChoice::GetSelectedAsset() const
{
    return _asset.lock();
}

bool DropdownAssetChoice::Initialize()
{
    if (!DropdownTextChoice::Initialize())
    {
        return false;
    }
    
    InitializeFromAssetInternal(_asset.lock());

    return true;
}

bool DropdownAssetChoice::InitializeFromAssetInternal(const SharedObjectPtr<Asset>& asset)
{
    _asset = asset;

    if (asset != nullptr)
    {
        SetText(asset->GetName().ToString());
    }
    else
    {
        SetText(L"None");
    }

    return true;
}

SharedObjectPtr<AssetPicker> AssetPicker::CreateForType(Type* assetType)
{
    if (assetType == nullptr)
    {
        DEBUG_BREAK();
        return nullptr;
    }

    const SharedObjectPtr<AssetPicker> assetPicker = NewObject<AssetPicker>();
    assetPicker->InitializeFromAssetType(assetType);
    return assetPicker;
}

void AssetPicker::InitializeFromAssetType(Type* assetType)
{
    if (assetType == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    if (!Initialize())
    {
        return;
    }

    _assetType = assetType;

    AssetManager& assetManager = AssetManager::Get();

    const SharedObjectPtr<DropdownAssetChoice> nullChoice = NewObject<DropdownAssetChoice>();
    nullChoice->InitializeFromAsset(nullptr);
    AddChoice(nullChoice);

    assetManager.ForEachAssetOfType(assetType,
                                    [this](const SharedObjectPtr<Asset>& asset)
                                    {
                                        const SharedObjectPtr<DropdownAssetChoice> choice = NewObject<
                                            DropdownAssetChoice>();
                                        choice->InitializeFromAsset(asset);
                                        AddChoice(choice);

                                        return true;
                                    },
                                    true);
}

Type* AssetPicker::GetAssetType() const
{
    return _assetType;
}

SharedObjectPtr<Asset> AssetPicker::GetSelectedAsset() const
{
    const SharedObjectPtr<DropdownAssetChoice> selectedWidget = GetSelectedChoice<DropdownAssetChoice>();
    if (selectedWidget == nullptr)
    {
        return nullptr;
    }

    return selectedWidget->GetSelectedAsset();
}

void AssetPicker::SetSelectedAsset(const SharedObjectPtr<Asset>& asset)
{
    for (const SharedObjectPtr<Widget>& widget : GetChoices())
    {
        const SharedObjectPtr<DropdownAssetChoice> choice = std::dynamic_pointer_cast<DropdownAssetChoice>(widget);
        if (choice == nullptr)
        {
            continue;
        }

        if (choice->GetSelectedAsset() == asset)
        {
            SetSelectedChoice(choice);
            return;
        }
    }
}
