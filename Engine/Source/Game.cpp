#include "Game.h"
#include "Engine/Subsystems/GameplaySubsystem.h"
#include "Engine/Subsystems/InputSubsystem.h"
#include "Rendering/Window.h"
#include "Rendering/Widgets/ViewportWidget.h"

bool Game::Startup(PassKey<GameplaySubsystem>)
{
    _isRunning = true;

    return OnStartup();
}

void Game::InitializeWorld(World& world, PassKey<GameplaySubsystem>)
{
    OnInitializeWorld(world);
}

void Game::Shutdown(PassKey<GameplaySubsystem>)
{
    _isRunning = false;

    OnShutdown();
}

bool Game::IsRunning() const
{
    return _isRunning;
}

std::shared_ptr<EntityTemplate> Game::GetPlayerTemplate() const
{
    return _playerTemplate;
}

void Game::OnInitializeWorld(World& world)
{
}

bool Game::OnStartup()
{
    if (_autoFocusViewport)
    {
        GameplaySubsystem::Get().GetMainViewport()->SetFocused(true);
    }

    return true;
}

void Game::OnShutdown()
{
    std::shared_ptr<ViewportWidget> viewport = GameplaySubsystem::Get().GetMainViewport();
    viewport->SetFocused(false);
    viewport->SetCamera(nullptr);
}

void Game::SetShouldAutoFocusViewportOnStartup(bool value)
{
    _autoFocusViewport = value;
}

bool Game::ShouldAutoFocusViewportOnStartup() const
{
    return _autoFocusViewport;
}
