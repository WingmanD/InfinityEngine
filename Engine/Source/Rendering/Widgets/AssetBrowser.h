#pragma once

#include "Widget.h"
#include "AssetBrowser.reflection.h"

class FlowBox;

REFLECTED()
class AssetBrowserEntry : public Widget
{
    GENERATED()

public:
    AssetBrowserEntry() = default;
    
    void InitializeFromAsset(const std::shared_ptr<Asset>& asset);

    std::shared_ptr<Asset> GetAsset() const;

    // Widget
public:
    bool Initialize() override;

private:
    std::weak_ptr<FlowBox> _horizontalBox;
    
    std::weak_ptr<Asset> _asset;

private:
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
    std::weak_ptr<FlowBox> _verticalBox;

private:
    void AddEntry(const std::shared_ptr<Asset>& asset) const;
};
