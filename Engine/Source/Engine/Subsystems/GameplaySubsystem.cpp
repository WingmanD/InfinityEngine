#include "GameplaySubsystem.h"
#include "Game.h"
#include "ProjectSettings.h"

bool GameplaySubsystem::StartGame()
{
    const Type* gameType = ProjectSettings::Get()->GetGameType();
    if (gameType == nullptr)
    {
        LOG(L"Could not start game - game type is not set in project settings.");
        return false;
    }

    _game = gameType->NewObject<Game>();

    return _game->Startup({});
}

void GameplaySubsystem::StopGame()
{
    if (_game == nullptr)
    {
        return;
    }
    
    if (!_game->IsRunning())
    {
        return;
    }

    _game->Shutdown({});
    _game = nullptr;
}

std::shared_ptr<Game> GameplaySubsystem::GetGame() const
{
    return _game;
}

bool GameplaySubsystem::Initialize()
{
    if (!EngineSubsystem::Initialize())
    {
        return false;
    }

    World* mainWorld = _worlds.AddDefault();
    return mainWorld->IsValid();
}

void GameplaySubsystem::Tick(double deltaTime)
{
    _worlds.ForEach([deltaTime](World& world)
    {
        world.Tick(deltaTime, {});
        return true;
    });
}

void GameplaySubsystem::Shutdown()
{
    StopGame();

    // todo we need to shut down some worlds on StopGame as well
    _worlds.ForEach([](World& world)
    {
        world.Shutdown();
        return true;
    });
}
