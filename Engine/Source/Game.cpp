#include "Game.h"
#include "ECS/EntityTemplate.h"
#include "ECS/Components/CCamera.h"
#include "ECS/Components/CStaticMesh.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/PathfindingSystem.h"
#include "ECS/Systems/StaticMeshRenderingSystem.h"
#include "Engine/Subsystems/GameplaySubsystem.h"
#include "Rendering/Widgets/ViewportWidget.h"

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
    templateAsset->Load();
    
    GameplaySubsystem& gameplaySubsystem = GameplaySubsystem::Get();
    gameplaySubsystem.GetWorlds().ForEach([&templateAsset, &gameplaySubsystem](World& world)
    {
        world.AddSystem<PathfindingSystem>();
        world.AddSystem<CameraSystem>();
        world.AddSystem<StaticMeshRenderingSystem>();
        
        Entity& entity = world.CreateEntity(templateAsset);
        gameplaySubsystem.GetMainViewport()->SetCamera(&entity.Get<CCamera>(templateAsset->GetArchetype()));
        
        return false;
    });

    return true;
}

void Game::OnShutdown()
{
    LOG(L"Game ended!");
}
