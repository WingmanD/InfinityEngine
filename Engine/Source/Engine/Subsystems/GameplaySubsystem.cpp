#include "GameplaySubsystem.h"
#include "Game.h"
#include "ProjectSettings.h"
#include "Engine/Engine.h"
#include "Rendering/Widgets/ViewportWidget.h"

GameplaySubsystem& GameplaySubsystem::Get()
{
    return Engine::Get().GetGameplaySubsystem();
}

bool GameplaySubsystem::StartGame(const std::shared_ptr<ViewportWidget>& viewport, const std::shared_ptr<Game>& game /*= nullptr*/)
{
    if (_game != nullptr && _game->IsRunning())
    {
        LOG(L"Could not start game - game is already running.");
        return false;
    }
    
    if (game == nullptr)
    {
        std::shared_ptr<ProjectSettings> projectSettings = ProjectSettings::Get();

        const std::shared_ptr<Game>& projectGame = projectSettings->GetGame();
        if (projectGame == nullptr)
        {
            const Type* gameType = projectSettings->GetGameType();
            if (gameType == nullptr)
            {
                LOG(L"Could not start game - game type is not set in project settings.");
                return false;
            }
            _game = gameType->NewObject<Game>();
        }
        else
        {
            _game = projectGame->DuplicateObject<Game>();
        }
    }
    else
    {
        _game = game;
    }

    _viewport = viewport;

    World* newWorld = _worlds.AddDefault();
    newWorld->Initialize({});

    _game->InitializeWorld(*newWorld, {});

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

    return true;
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
