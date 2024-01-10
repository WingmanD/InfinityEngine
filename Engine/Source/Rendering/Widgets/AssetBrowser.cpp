#include "AssetBrowser.h"

#include "AssetCreatorMenu.h"
#include "Button.h"
#include "FlowBox.h"
#include "TextBox.h"
#include "Engine/Subsystems/AssetManager.h"
#include "Rendering/Window.h"

void AssetBrowserEntry::InitializeFromAsset(const std::shared_ptr<Asset>& asset)
{
    const std::shared_ptr<FlowBox> horizontalBox = _horizontalBox.lock();

    const std::shared_ptr<TextBox> textBox = horizontalBox->AddChild<TextBox>();
    textBox->SetText(asset->GetName());

    const std::shared_ptr<Button> editButton = horizontalBox->AddChild<Button>();
    editButton->SetText(L"Edit");
    editButton->OnReleased.Add([asset, this]()
    {
        // todo once the rest of editor UI is completed, here we should open a new tab with the asset properties widget
        if (const std::shared_ptr<Window> window = GetParentWindow())
        {
            window->GetRootWidget()->AddChild(asset->GetType()->CreatePropertiesWidget(asset));
        }
    });

    const std::shared_ptr<Button> deleteButton = horizontalBox->AddChild<Button>();
    deleteButton->SetText(L"Delete");
    deleteButton->OnReleased.Add([asset, this]()
    {
        if (asset != nullptr)
        {
            AssetManager::Get().DeleteAsset(asset);
        }
    });
}

std::shared_ptr<Asset> AssetBrowserEntry::GetAsset() const
{
    return _asset.lock();
}

bool AssetBrowserEntry::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    const std::shared_ptr<FlowBox> horizontalBox = AddChild<FlowBox>();
    if (horizontalBox == nullptr)
    {
        return false;
    }
    _horizontalBox = horizontalBox;

    horizontalBox->SetDirection(EFlowBoxDirection::Horizontal);

    return true;
}

bool AssetBrowser::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    const auto verticalBox = AddChild<FlowBox>();
    if (verticalBox == nullptr)
    {
        return false;
    }
    verticalBox->SetDirection(EFlowBoxDirection::Vertical);

    const auto horizontalButtonBox = verticalBox->AddChild<FlowBox>();
    if (horizontalButtonBox == nullptr)
    {
        return false;
    }
    horizontalButtonBox->SetDirection(EFlowBoxDirection::Horizontal);

    auto newAssetButton = horizontalButtonBox->AddChild<Button>();
    newAssetButton->SetText(L"New Asset");
    newAssetButton->OnReleased.Add([this, newAssetButton]()
    {
        if (const std::shared_ptr<Window> window = GetParentWindow())
        {
            // todo layout adding to button for now because rendering order depends on tree depth
            auto assetCreator = newAssetButton->AddChild<AssetCreatorMenu>();
            if (assetCreator != nullptr)
            {
                newAssetButton->SetEnabled(false);
                newAssetButton->SetCollisionEnabled(false);

                assetCreator->OnDestroyed.Add([newAssetButton]()
                {
                    if (newAssetButton != nullptr)
                    {
                        newAssetButton->SetEnabled(true);
                        newAssetButton->SetCollisionEnabled(true);
                    }
                });
            }
        }
    });

    const auto importAssetButton = horizontalButtonBox->AddChild<Button>();
    importAssetButton->SetText(L"Import");
    importAssetButton->OnReleased.Add([this]()
    {
        if (const std::shared_ptr<Window> window = GetParentWindow())
        {
            // todo layout
            //window->GetRootWidget()->AddChild<AssetImportMenu>();
        }
    });

    const auto assetsVerticalBox = verticalBox->AddChild<FlowBox>();
    if (assetsVerticalBox == nullptr)
    {
        return false;
    }
    _verticalBox = assetsVerticalBox;
    assetsVerticalBox->SetDirection(EFlowBoxDirection::Vertical);

    AssetManager& assetManager = AssetManager::Get();
    assetManager.ForEachAsset([this](const std::shared_ptr<Asset>& asset)
    {
        AddEntry(asset);

        return true;
    });

    assetManager.OnAssetCreated.Add([this](const std::shared_ptr<Asset>& asset)
    {
        AddEntry(asset);
    });

    assetManager.OnAssetDeleted.Add([this](const std::shared_ptr<Asset>& asset)
    {
        const std::shared_ptr<FlowBox> verticalBox = _verticalBox.lock();
        for (const std::shared_ptr<Widget>& widget : verticalBox->GetChildren())
        {
            std::shared_ptr<AssetBrowserEntry> entry = std::dynamic_pointer_cast<AssetBrowserEntry>(widget);
            if (entry == nullptr)
            {
                continue;
            }

            if (entry->GetAsset() == asset)
            {
                verticalBox->RemoveChild(entry);
                break;
            }
        }
    });

    return true;
}

void AssetBrowser::AddEntry(const std::shared_ptr<Asset>& asset) const
{
    const std::shared_ptr<AssetBrowserEntry> entry = _verticalBox.lock()->AddChild<AssetBrowserEntry>();
    if (entry == nullptr)
    {
        return;
    }

    entry->InitializeFromAsset(asset);
}
