#include "Game.h"
#include "ECS/Components/CStaticMesh.h"
#include "ECS/Components/CTransform.h"
#include "ECS/EntityTemplate.h"
#include "ECS/Systems/PathfindingSystem.h"
#include "ECS/Systems/StaticMeshRenderingSystem.h"
#include "Engine/Subsystems/GameplaySubsystem.h"

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

    std::shared_ptr<EntityTemplate> templateAsset = AssetManager::Get().FindAssetByName<EntityTemplate>(
        Name(L"EntityTemplateTest"));
    
    GameplaySubsystem& gameplaySubsystem = GameplaySubsystem::Get();
    gameplaySubsystem.GetWorlds().ForEach([&templateAsset](World& world)
    {
        world.CreateEntity(templateAsset);
        
        world.AddSystem<PathfindingSystem>();
        world.AddSystem<StaticMeshRenderingSystem>();
        
        return true;
    });

    return true;
}

void Game::OnShutdown()
{
    LOG(L"Game ended!");
}
