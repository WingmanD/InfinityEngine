﻿#include "GameplaySubsystem.h"
#include "Game.h"
#include "ProjectSettings.h"
#include "Engine/Engine.h"
#include "Rendering/Widgets/ViewportWidget.h"

GameplaySubsystem& GameplaySubsystem::Get()
{
    return Engine::Get().GetGameplaySubsystem();
}

bool GameplaySubsystem::StartGame(const std::shared_ptr<ViewportWidget>& viewport)
{
    const Type* gameType = ProjectSettings::Get()->GetGameType();
    if (gameType == nullptr)
    {
        LOG(L"Could not start game - game type is not set in project settings.");
        return false;
    }

    _viewport = viewport;

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

    _worlds.ForEach([](World& world)
    {
        world.Shutdown({});
        return true;
    });

    _worlds.Clear();
}

std::shared_ptr<Game> GameplaySubsystem::GetGame() const
{
    return _game;
}

BucketArray<World>& GameplaySubsystem::GetWorlds()
{
    return _worlds;
}

std::shared_ptr<ViewportWidget> GameplaySubsystem::GetMainViewport() const
{
    return _viewport.lock();
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
    // todo this won't work for multiple worlds because we need to wait for all worlds to finish their async operations
    // also, we need to tick systems in parallel
    _worlds.ForEach([deltaTime](World& world)
    {
        world.Tick(deltaTime, {});
        return true;
    });
}

void GameplaySubsystem::Shutdown()
{
    StopGame();
}
