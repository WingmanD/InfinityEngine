#include "LevelEditorGame.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/FloatingControlSystem.h"
#include "ECS/Systems/LevelEditorSystem.h"
#include "ECS/Systems/PhysicsSystem.h"
#include "ECS/Systems/PointLightSystem.h"
#include "ECS/Systems/StaticMeshRenderingSystem.h"
#include "Engine/Subsystems/GameplaySubsystem.h"
#include "Rendering/Widgets/ViewportWidget.h"

LevelEditorGame::LevelEditorGame() : Game()
{
    SetShouldAutoFocusViewportOnStartup(false);
}

void LevelEditorGame::SetLevel(const SharedObjectPtr<Level>& level)
{
    _level = level;
}

SharedObjectPtr<Level> LevelEditorGame::GetLevel() const
{
    return _level;
}

void LevelEditorGame::OnInitializeWorld(World& world)
{
    FloatingControlSystem& controlSystem = world.AddSystem<FloatingControlSystem>();
    world.AddSystem<LevelEditorSystem>();
    world.AddSystem<CameraSystem>();
    world.AddSystem<PhysicsSystem>().SetSimulationEnabled(false);
    world.AddSystem<StaticMeshRenderingSystem>();
    world.AddSystem<PointLightSystem>();

    GameplaySubsystem& gameplaySubsystem = GameplaySubsystem::Get();
    if (gameplaySubsystem.GetMainViewport()->GetCamera() != nullptr)
    {
        return;
    }

    SharedObjectPtr<EntityTemplate> playerTemplate = GetPlayerTemplate();

    Entity& playerEntity = world.CreateEntity(playerTemplate);
    gameplaySubsystem.GetMainViewport()->SetCamera(&playerEntity.Get<CCamera>(playerTemplate->GetArchetype()));
    Transform& cameraEntityTransform = playerEntity.Get<CTransform>(playerTemplate->GetArchetype()).ComponentTransform;
    cameraEntityTransform.SetWorldLocation({-5.0f, 0.0f, 1.0f});

    controlSystem.TakeControlOf(playerEntity);
}
