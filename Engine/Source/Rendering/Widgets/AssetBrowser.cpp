#include "AssetBrowser.h"

#include "AssetCreatorMenu.h"
#include "Button.h"
#include "FlowBox.h"
#include "TextBox.h"
#include "Engine/Subsystems/AssetManager.h"
#include "Rendering/Window.h"

bool AssetBrowserEntry::InitializeFromAsset(const std::shared_ptr<Asset>& asset)
{
    if (!Initialize())
    {
        return false;
    }

    const std::shared_ptr<TextBox> textBox = AddChild<TextBox>();
    if (textBox == nullptr)
    {
        return false;
    }
    textBox->SetText(asset->GetName());
    textBox->SetCollisionEnabled(true);

    const std::shared_ptr<Button> editButton = AddChild<Button>();
    if (editButton == nullptr)
    {
        return false;
    }
    editButton->SetText(L"Edit");
    editButton->OnReleased.Add([asset, this]()
    {
        // todo once the rest of editor UI is completed, here we should open a new tab with the asset properties widget
        if (const std::shared_ptr<Window> window = GetParentWindow())
        {
            window->GetRootWidget()->AddChild(asset->GetType()->CreatePropertiesWidget(asset));
        }
    });

    const std::shared_ptr<Button> deleteButton = AddChild<Button>();
    if (deleteButton == nullptr)
    {
        return false;
    }
    deleteButton->SetText(L"Delete");
    deleteButton->OnReleased.Add([asset, this]()
    {
        if (asset != nullptr)
        {
            AssetManager::Get().DeleteAsset(asset);
        }
    });

    _asset = asset;

    return true;
}

std::shared_ptr<Asset> AssetBrowserEntry::GetAsset() const
{
    return _asset.lock();
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
            auto assetCreator = newAssetButton->GetTextBox()->AddChild<AssetCreatorMenu>();
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

    const auto assetsVerticalBox = verticalBox->AddChild<TableWidget>();
    if (assetsVerticalBox == nullptr)
    {
        return false;
    }
    _table = assetsVerticalBox;

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
        const std::shared_ptr<TableWidget> verticalBox = _table.lock();
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
    std::shared_ptr<AssetBrowserEntry> row = std::make_shared<AssetBrowserEntry>();
    row->InitializeFromAsset(asset);
    
    _table.lock()->AddRow(row);
}
