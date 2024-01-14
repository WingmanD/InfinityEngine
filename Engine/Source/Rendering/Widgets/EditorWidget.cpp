#include "EditorWidget.h"
#include "AssetBrowser.h"
#include "CanvasPanel.h"

bool EditorWidget::Initialize()
{
    if (!TabSwitcher::Initialize())
    {
        return false;
    }

    SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);

    const std::shared_ptr<CanvasPanel> assetBrowserPanel = AddTab<CanvasPanel>(L"Infinity Engine", false);
    if (!assetBrowserPanel)
    {
        return false;
    }

    assetBrowserPanel->SetVisibility(false);
    assetBrowserPanel->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);

    const std::shared_ptr<AssetBrowser> assetBrowser = assetBrowserPanel->AddChild<AssetBrowser>();
    if (!assetBrowser)
    {
        return false;
    }

    assetBrowser->SetAnchor(EWidgetAnchor::TopLeft);
    assetBrowser->SetSelfAnchor(EWidgetAnchor::TopLeft);
    
    return true;
}
