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

    GameplaySubsystem& gameplaySubsystem = GameplaySubsystem::Get();
    gameplaySubsystem.GetWorlds().ForEach([&gameplaySubsystem, this](World& world)
    {
        world.AddSystem<PathfindingSystem>();
        world.AddSystem<CameraSystem>();
        world.AddSystem<StaticMeshRenderingSystem>();

        Entity& entity = world.CreateEntity(_playerTemplate);
        gameplaySubsystem.GetMainViewport()->SetCamera(&entity.Get<CCamera>(_playerTemplate->GetArchetype()));
        Transform& cameraEntityTransform = entity.Get<CTransform>(_enemyTemplate->GetArchetype()).ComponentTransform;
        cameraEntityTransform.SetWorldLocation({-5.0f, 0.0f, 1.0f});

        world.CreateEntity(_enemyTemplate);

        Entity& meshEntity = world.CreateEntity(_enemyTemplate);
        Transform& transform = meshEntity.Get<CTransform>(_enemyTemplate->GetArchetype()).ComponentTransform;
        transform.SetWorldRotation({0.0f, 0.0f, 45.0f});
        
        world.CreateEntity(_cubeTemplate);
        world.CreateEntity(_enemyTemplate);
        world.CreateEntity(_cubeTemplate);
        world.CreateEntity(_enemyTemplate);

        return false;
    });

    return true;
}

void Game::OnShutdown()
{
    LOG(L"Game ended!");
}
