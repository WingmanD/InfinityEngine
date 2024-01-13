#pragma once

#include "Widget.h"
#include "TableWidget.h"
#include "AssetBrowser.reflection.h"

REFLECTED()
class AssetBrowserEntry : public TableRowWidget
{
    GENERATED()

public:
    AssetBrowserEntry() = default;
    
    bool InitializeFromAsset(const std::shared_ptr<Asset>& asset);

    std::shared_ptr<Asset> GetAsset() const;

private:
    std::weak_ptr<Asset> _asset;
};

REFLECTED()
class AssetBrowser : public Widget
{
    GENERATED()

public:
    AssetBrowser() = default;

    // Widget
public:
    bool Initialize() override;
    
private:
    std::weak_ptr<TableWidget> _table;

private:
    void AddEntry(const std::shared_ptr<Asset>& asset) const;
};
