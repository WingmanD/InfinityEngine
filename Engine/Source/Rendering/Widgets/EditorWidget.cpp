#include "EditorWidget.h"
#include "AssetBrowser.h"
#include "Button.h"
#include "CanvasPanel.h"
#include "Game.h"
#include "Engine/Engine.h"

bool EditorWidget::Initialize()
{
    if (!TabSwitcher::Initialize())
    {
        return false;
    }

    SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);

    const std::shared_ptr<CanvasPanel> mainTab = AddTab<CanvasPanel>(L"Infinity Engine", false);
    if (!mainTab)
    {
        return false;
    }

    mainTab->SetVisibility(false);
    mainTab->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);

    const std::shared_ptr<AssetBrowser> assetBrowser = mainTab->AddChild<AssetBrowser>();
    if (!assetBrowser)
    {
        return false;
    }

    assetBrowser->SetAnchor(EWidgetAnchor::TopLeft);
    assetBrowser->SetSelfAnchor(EWidgetAnchor::TopLeft);

    std::shared_ptr<Button> playButton = mainTab->AddChild<Button>();
    if (!playButton)
    {
        return false;
    }

    playButton->SetText(L"Play");
    playButton->SetAnchor(EWidgetAnchor::TopCenter);
    playButton->SetSelfAnchor(EWidgetAnchor::TopCenter);

    playButton->OnReleased.Add([playButton]()
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
        
        if (gameplaySubsystem.StartGame())
        {
            playButton->SetText(L"Stop");
        }
    });

    return true;
}
