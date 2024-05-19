#include "Game.h"
#include "ECS/EntityTemplate.h"
#include "ECS/Components/CCamera.h"
#include "ECS/Components/CStaticMesh.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/PathfindingSystem.h"
#include "ECS/Systems/PhysicsSystem.h"
#include "ECS/Systems/PointLightSystem.h"
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
        world.AddSystem<PhysicsSystem>();
        world.AddSystem<StaticMeshRenderingSystem>();
        world.AddSystem<PointLightSystem>();

        Entity& entity = world.CreateEntity(_playerTemplate);
        gameplaySubsystem.GetMainViewport()->SetCamera(&entity.Get<CCamera>(_playerTemplate->GetArchetype()));
        Transform& cameraEntityTransform = entity.Get<CTransform>(_playerTemplate->GetArchetype()).ComponentTransform;
        cameraEntityTransform.SetWorldLocation({-5.0f, 0.0f, 1.0f});

        world.CreateEntity(_enemyTemplate);
        world.CreateEntity(_floorTemplate);

        // Entity& meshEntity = world.CreateEntity(_enemyTemplate);
        // Transform& transform = meshEntity.Get<CTransform>(_enemyTemplate->GetArchetype()).ComponentTransform;
        // transform.SetWorldRotation({0.0f, 0.0f, 45.0f});
        // transform.SetWorldLocation({0.0f, 0.0f, 1.0f});
        //
        // Entity& cubeEntity = world.CreateEntity(_cubeTemplate);
        // Transform& cubeTransform = cubeEntity.Get<CTransform>(_cubeTemplate->GetArchetype()).ComponentTransform;
        // cubeTransform.SetWorldScale({0.25f, 0.25f, 0.25f});
        //
        // Entity& meshEntity2 = world.CreateEntity(_enemyTemplate);
        // Transform& transform2 = meshEntity2.Get<CTransform>(_enemyTemplate->GetArchetype()).ComponentTransform;
        // transform2.SetWorldLocation({0.0f, 0.0f, 2.0f});
        // world.CreateEntity(_cubeTemplate);
        // world.CreateEntity(_enemyTemplate);

        return false;
    });

    return true;
}

void Game::OnShutdown()
{
    LOG(L"Game ended!");
}
