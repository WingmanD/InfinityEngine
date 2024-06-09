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

    const std::shared_ptr<FlowBox> mainTab = AddTab<FlowBox>(L"Infinity Engine", false);
    if (!mainTab)
    {
        return false;
    }

    mainTab->SetVisibility(false);
    mainTab->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);
    mainTab->SetDirection(EFlowBoxDirection::Vertical);

    const std::shared_ptr<FlowBox> toolbar = mainTab->AddChild<FlowBox>();
    if (toolbar == nullptr)
    {
        return false;
    }

    std::shared_ptr<Button> playButton = toolbar->AddChild<Button>();
    if (!playButton)
    {
        return false;
    }

    playButton->SetText(L"Play");

    const std::shared_ptr<FlowBox> horizontalBox = mainTab->AddChild<FlowBox>();
    if (horizontalBox == nullptr)
    {
        return false;
    }

    horizontalBox->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);
    horizontalBox->SetDirection(EFlowBoxDirection::Horizontal);

    const std::shared_ptr<AssetBrowser> assetBrowser = horizontalBox->AddChild<AssetBrowser>();
    if (!assetBrowser)
    {
        return false;
    }

    assetBrowser->SetFillMode(EWidgetFillMode::FillY);

    const std::shared_ptr<ViewportWidget> viewport = horizontalBox->AddChild<ViewportWidget>();
    if (viewport == nullptr)
    {
        return false;
    }

    viewport->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);

    std::ignore = playButton->OnReleased.Add([playButton, viewport]()
    {
        GameplaySubsystem& gameplaySubsystem = Engine::Get().GetGameplaySubsystem();

        if (const std::shared_ptr<Game> game = gameplaySubsystem.GetGame())
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
