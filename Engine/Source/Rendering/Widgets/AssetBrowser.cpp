#include "AssetBrowser.h"

#include "AssetCreatorMenu.h"
#include "AssetImportMenu.h"
#include "Button.h"
#include "EditorWidget.h"
#include "FlowBox.h"
#include "TextBox.h"
#include "Engine/Subsystems/AssetManager.h"
#include "Rendering/Window.h"

bool AssetBrowserEntryBase::InitializeFromAsset(const SharedObjectPtr<Asset>& asset)
{
    if (!Initialize())
    {
        return false;
    }

    _asset = asset;

    return true;
}

SharedObjectPtr<Asset> AssetBrowserEntryBase::GetAsset() const
{
    return _asset.lock();
}

bool AssetBrowserEntry::InitializeFromAsset(const SharedObjectPtr<Asset>& asset)
{
    if (!AssetBrowserEntryBase::InitializeFromAsset(asset))
    {
        return false;
    }

    constexpr Vector4 textPadding = {4.0f, 4.0f, 1.0f, 1.0f};

    const SharedObjectPtr<TextBox> textBox = AddChild<TextBox>();
    if (textBox == nullptr)
    {
        return false;
    }
    textBox->SetText(asset->GetName().ToString());
    textBox->SetPadding({4.0f, 10.0f, 1.0f, 1.0f});
    textBox->SetCollisionEnabled(true);

    const SharedObjectPtr<Button> editButton = AddChild<Button>();
    if (editButton == nullptr)
    {
        return false;
    }
    editButton->SetText(L"Edit");
    editButton->GetTextBox()->SetPadding(textPadding);
    std::ignore = editButton->OnReleased.Add([asset, this]()
    {
        if (const SharedObjectPtr<EditorWidget> editorWidget = std::dynamic_pointer_cast<EditorWidget>(
            GetRootWidget()->GetChildren()[0]))
        {
            if (!asset->Load())
            {
                LOG(L"Failed to open asset for editing: {}", asset->GetName().ToString());
                return;
            }

            const SharedObjectPtr<Widget> editWidget = asset->CreateEditWidget();
            if (editWidget == nullptr)
            {
                DEBUG_BREAK();
                return;
            }

            editWidget->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);

            editorWidget->AddTab(asset->GetName().ToString(), editWidget);
            editorWidget->SetTabIndex(editorWidget->GetTabCount());
        }
    });

    const SharedObjectPtr<Button> deleteButton = AddChild<Button>();
    if (deleteButton == nullptr)
    {
        return false;
    }
    deleteButton->SetText(L"Delete");
    deleteButton->GetTextBox()->SetPadding(textPadding);
    std::ignore = deleteButton->OnReleased.Add([asset, this]()
    {
        if (asset != nullptr)
        {
            AssetManager::Get().DeleteAsset(asset);
        }
    });

    return true;
}

void AssetBrowser::SetEntryType(const SubtypeOf<AssetBrowserEntryBase>& entryType)
{
    _entryType = entryType;
}

Type& AssetBrowser::GetEntryType() const
{
    return *_entryType;
}

void AssetBrowser::SetFilter(std::function<bool(const Asset& asset)>&& filter)
{
    _filter = std::move(filter);
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
    std::ignore = newAssetButton->OnReleased.Add([this, newAssetButton]()
    {
        if (const std::shared_ptr<Window> window = GetParentWindow())
        {
            const SharedObjectPtr<Widget> assetCreator = window->AddPopup<AssetCreatorMenu>();
            if (assetCreator != nullptr)
            {
                newAssetButton->SetEnabled(false);
                newAssetButton->SetCollisionEnabled(false);

                std::ignore = assetCreator->OnDestroyed.Add([newAssetButton]()
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
    std::ignore = importAssetButton->OnReleased.Add([this, importAssetButton]()
    {
        if (const std::shared_ptr<Window> window = GetParentWindow())
        {
            const SharedObjectPtr<Widget> importer = window->AddPopup<AssetImportMenu>();
            if (importer != nullptr)
            {
                importAssetButton->SetEnabled(false);
                importAssetButton->SetCollisionEnabled(false);

                std::ignore = importer->OnDestroyed.Add([importAssetButton]()
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
    assetManager.ForEachAsset([this](const SharedObjectPtr<Asset>& asset)
    {
        AddEntry(asset);

        return true;
    });

    std::ignore = assetManager.OnAssetCreated.Add([this](const SharedObjectPtr<Asset>& asset)
    {
        AddEntry(asset);
    });

    std::ignore = assetManager.OnAssetDeleted.Add([this](const SharedObjectPtr<Asset>& asset)
    {
        for (const SharedObjectPtr<Widget>& widget : GetTable()->GetChildren()[0]->GetChildren())
        {
            SharedObjectPtr<AssetBrowserEntry> entry = std::dynamic_pointer_cast<AssetBrowserEntry>(widget);
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

void AssetBrowser::AddEntry(const SharedObjectPtr<Asset>& asset) const
{
    if (!_filter(*asset))
    {
        return;
    }

    const SharedObjectPtr<AssetBrowserEntryBase> row = _entryType->NewObject<AssetBrowserEntryBase>();
    row->InitializeFromAsset(asset);

    GetTable()->AddRow(row);
}

SharedObjectPtr<TableWidget> AssetBrowser::GetTable() const
{
    return _table.lock();
}
