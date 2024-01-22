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

bool DropdownAssetChoice::InitializeFromAsset(const std::shared_ptr<Asset>& asset)
{
    if (!Initialize())
    {
        return false;
    }

    return InitializeFromAssetInternal(asset);
}

std::shared_ptr<Asset> DropdownAssetChoice::GetSelectedAsset() const
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

bool DropdownAssetChoice::InitializeFromAssetInternal(const std::shared_ptr<Asset>& asset)
{
    _asset = asset;

    const std::shared_ptr<TextBox> textWidget = GetTextBox();

    if (asset != nullptr)
    {
        textWidget->SetText(asset->GetName());
    }
    else
    {
        textWidget->SetText(L"None");
    }

    return true;
}

std::shared_ptr<AssetPicker> AssetPicker::CreateForType(Type* assetType)
{
    if (assetType == nullptr)
    {
        DEBUG_BREAK();
        return nullptr;
    }

    const std::shared_ptr<AssetPicker> assetPicker = std::make_shared<AssetPicker>();
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

    const std::shared_ptr<DropdownAssetChoice> nullChoice = std::make_shared<DropdownAssetChoice>();
    nullChoice->InitializeFromAsset(nullptr);
    AddChoice(nullChoice);

    assetManager.ForEachAssetOfType(assetType,
                                    [this](const std::shared_ptr<Asset>& asset)
                                    {
                                        const std::shared_ptr<DropdownAssetChoice> choice = std::make_shared<
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

std::shared_ptr<Asset> AssetPicker::GetSelectedAsset() const
{
    const std::shared_ptr<DropdownAssetChoice> selectedWidget = GetSelectedChoice<DropdownAssetChoice>();
    if (selectedWidget == nullptr)
    {
        return nullptr;
    }

    return selectedWidget->GetSelectedAsset();
}

void AssetPicker::SetSelectedAsset(const std::shared_ptr<Asset>& asset)
{
    for (const std::shared_ptr<Widget>& widget : GetChoices())
    {
        const std::shared_ptr<DropdownAssetChoice> choice = std::dynamic_pointer_cast<DropdownAssetChoice>(widget);
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
