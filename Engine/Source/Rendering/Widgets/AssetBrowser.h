#pragma once

#include "SubtypeOf.h"
#include "TableWidget.h"
#include "Widget.h"
#include "AssetBrowser.reflection.h"

REFLECTED()
class AssetBrowserEntryBase : public TableRowWidget
{
    GENERATED()

public:
    AssetBrowserEntryBase() = default;

    virtual bool InitializeFromAsset(const SharedObjectPtr<Asset>& asset);

    SharedObjectPtr<Asset> GetAsset() const;

private:
    std::weak_ptr<Asset> _asset;
};

REFLECTED()
class AssetBrowserEntry : public AssetBrowserEntryBase
{
    GENERATED()

public:
    AssetBrowserEntry() = default;

    // AssetBrowserEntryBase
public:
    virtual bool InitializeFromAsset(const SharedObjectPtr<Asset>& asset) override;
};

REFLECTED()

class AssetBrowser : public Widget
{
    GENERATED()

public:
    AssetBrowser() = default;

    void SetEntryType(const SubtypeOf<AssetBrowserEntryBase>& entryType);
    Type& GetEntryType() const;

    void SetFilter(std::function<bool(const Asset& asset)>&& filter);

    // Widget
public:
    bool Initialize() override;

protected:
    virtual void AddEntry(const SharedObjectPtr<Asset>& asset) const;

    [[nodiscard]] SharedObjectPtr<TableWidget> GetTable() const;

private:
    std::weak_ptr<TableWidget> _table;

    SubtypeOf<AssetBrowserEntryBase> _entryType = AssetBrowserEntry::StaticType();
    std::function<bool(const Asset& asset)> _filter = [](const Asset&) { return true; };
};
