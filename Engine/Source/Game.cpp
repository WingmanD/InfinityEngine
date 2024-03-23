#include "Game.h"

bool Game::Startup(PassKey<GameplaySubsystem>)
{
    _isRunning = true;

    return OnStartup();
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

bool Game::OnStartup()
{
    LOG(L"Game started!");

    return true;
}

void Game::OnShutdown()
{
    LOG(L"Game ended!");
}
