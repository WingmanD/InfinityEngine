#include "AssetBrowser.h"

#include "AssetCreatorMenu.h"
#include "AssetImportMenu.h"
#include "Button.h"
#include "EditorWidget.h"
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

    constexpr Vector4 textPadding = {4.0f, 4.0f, 1.0f, 1.0f};

    const std::shared_ptr<TextBox> textBox = AddChild<TextBox>();
    if (textBox == nullptr)
    {
        return false;
    }
    textBox->SetText(asset->GetName());
    textBox->SetPadding({4.0f, 10.0f, 1.0f, 1.0f});
    textBox->SetCollisionEnabled(true);

    const std::shared_ptr<Button> editButton = AddChild<Button>();
    if (editButton == nullptr)
    {
        return false;
    }
    editButton->SetText(L"Edit");
    editButton->GetTextBox()->SetPadding(textPadding);
    editButton->OnReleased.Add([asset, this]()
    {
        if (const std::shared_ptr<Window> window = GetParentWindow())
        {
            if (const std::shared_ptr<EditorWidget> editorWidget = std::dynamic_pointer_cast<EditorWidget>(
                GetRootWidget()->GetChildren()[0]))
            {
                const std::shared_ptr<Widget> propertiesWidget = asset->GetType()->CreatePropertiesWidget(asset);
                if (propertiesWidget == nullptr)
                {
                    DEBUG_BREAK();
                    return;
                }

                propertiesWidget->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);

                editorWidget->AddTab(asset->GetName(), propertiesWidget);
                editorWidget->SetTabIndex(editorWidget->GetTabCount());
            }
        }
    });

    const std::shared_ptr<Button> deleteButton = AddChild<Button>();
    if (deleteButton == nullptr)
    {
        return false;
    }
    deleteButton->SetText(L"Delete");
    deleteButton->GetTextBox()->SetPadding(textPadding);
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

    constexpr Vector4 textPadding = {4.0f, 4.0f, 1.0f, 1.0f};

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
    newAssetButton->GetTextBox()->SetPadding(textPadding);
    newAssetButton->OnReleased.Add([this, newAssetButton]()
    {
        if (const std::shared_ptr<Window> window = GetParentWindow())
        {
            const std::shared_ptr<Widget> assetCreator = window->AddPopup<AssetCreatorMenu>();
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
    importAssetButton->GetTextBox()->SetPadding(textPadding);
    importAssetButton->OnReleased.Add([this, importAssetButton]()
    {
        if (const std::shared_ptr<Window> window = GetParentWindow())
        {
            const std::shared_ptr<Widget> importer = window->AddPopup<AssetImportMenu>();
            if (importer != nullptr)
            {
                importAssetButton->SetEnabled(false);
                importAssetButton->SetCollisionEnabled(false);

                importer->OnDestroyed.Add([importAssetButton]()
                {
                    if (importAssetButton != nullptr)
                    {
                        importAssetButton->SetEnabled(true);
                        importAssetButton->SetCollisionEnabled(true);
                    }
                });
            }
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
        const std::shared_ptr<TableWidget> table = _table.lock();
        for (const std::shared_ptr<Widget>& widget : table->GetChildren()[0]->GetChildren())
        {
            std::shared_ptr<AssetBrowserEntry> entry = std::dynamic_pointer_cast<AssetBrowserEntry>(widget);
            if (entry == nullptr)
            {
                continue;
            }

            if (entry->GetAsset() == asset)
            {
                entry->DestroyWidget();
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
