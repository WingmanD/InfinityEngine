#include "DemoGame.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/FloatingControlSystem.h"
#include "ECS/Systems/LevelStreamingSystem.h"
#include "ECS/Systems/PathfindingSystem.h"
#include "ECS/Systems/PhysicsSystem.h"
#include "ECS/Systems/PointLightSystem.h"
#include "ECS/Systems/StaticMeshRenderingSystem.h"
#include "Engine/Subsystems/GameplaySubsystem.h"
#include "Rendering/Widgets/ViewportWidget.h"

void DemoGame::OnInitializeWorld(World& world)
{
    world.AddSystem<PathfindingSystem>();
    FloatingControlSystem& controlSystem = world.AddSystem<FloatingControlSystem>();
    world.AddSystem<CameraSystem>();
    world.AddSystem<PhysicsSystem>();
    world.AddSystem<StaticMeshRenderingSystem>();
    world.AddSystem<PointLightSystem>();
    world.AddSystem<LevelStreamingSystem>().SetLevel(_level);

    GameplaySubsystem& gameplaySubsystem = GameplaySubsystem::Get();
    if (gameplaySubsystem.GetMainViewport()->GetCamera() != nullptr)
    {
        return;
    }

    std::shared_ptr<EntityTemplate> playerTemplate = GetPlayerTemplate();
    const Archetype& playerArchetype = playerTemplate->GetArchetype();

    Entity& playerEntity = world.CreateEntity(playerTemplate);
    gameplaySubsystem.GetMainViewport()->SetCamera(&playerEntity.Get<CCamera>(playerArchetype));

    Transform& cameraEntityTransform = playerEntity.Get<CTransform>(playerArchetype).ComponentTransform;
    cameraEntityTransform.SetWorldLocation({-5.0f, 0.0f, 1.0f});

    controlSystem.TakeControlOf(playerEntity);
}
