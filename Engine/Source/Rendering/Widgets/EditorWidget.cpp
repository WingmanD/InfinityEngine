#include "EditorWidget.h"
#include "AssetBrowser.h"
#include "Button.h"
#include "FlowBox.h"
#include "Game.h"
#include "ViewportWidget.h"
#include "Engine/Engine.h"

bool EditorWidget::Initialize()
{
    if (!TabSwitcher::Initialize())
    {
        return false;
    }

    SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);

    const SharedObjectPtr<FlowBox> mainTab = AddTab<FlowBox>(L"Infinity Engine", false);
    if (!mainTab)
    {
        return false;
    }

    mainTab->SetVisibility(false);
    mainTab->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);
    mainTab->SetDirection(EFlowBoxDirection::Vertical);

    const SharedObjectPtr<FlowBox> toolbar = mainTab->AddChild<FlowBox>();
    if (toolbar == nullptr)
    {
        return false;
    }

    SharedObjectPtr<Button> playButton = toolbar->AddChild<Button>();
    if (!playButton)
    {
        return false;
    }

    playButton->SetText(L"Play");

    const SharedObjectPtr<FlowBox> horizontalBox = mainTab->AddChild<FlowBox>();
    if (horizontalBox == nullptr)
    {
        return false;
    }

    horizontalBox->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);
    horizontalBox->SetDirection(EFlowBoxDirection::Horizontal);

    const SharedObjectPtr<AssetBrowser> assetBrowser = horizontalBox->AddChild<AssetBrowser>();
    if (!assetBrowser)
    {
        return false;
    }

    assetBrowser->SetFillMode(EWidgetFillMode::FillY);

    const SharedObjectPtr<ViewportWidget> viewport = horizontalBox->AddChild<ViewportWidget>();
    if (viewport == nullptr)
    {
        return false;
    }

    viewport->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);

    std::ignore = playButton->OnReleased.Add([playButton, viewport]()
    {
        GameplaySubsystem& gameplaySubsystem = Engine::Get().GetGameplaySubsystem();

        if (const SharedObjectPtr<Game> game = gameplaySubsystem.GetGame())
        {
            if (game->IsRunning())
            {
                gameplaySubsystem.StopGame();

                playButton->SetText(L"Play");

                return;
            }
        }

        if (gameplaySubsystem.StartGame(viewport))
        {
            playButton->SetText(L"Stop");
        }
    });

    return true;
}
