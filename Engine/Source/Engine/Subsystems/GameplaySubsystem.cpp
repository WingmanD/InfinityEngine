#include "GameplaySubsystem.h"
#include "ECS/Components/CStaticMesh.h"
#include "ECS/Components/CTransform.h"

bool GameplaySubsystem::Initialize()
{
    if (!EngineSubsystem::Initialize())
    {
        return false;
    }

    World* mainWorld = _worlds.AddDefault();
    mainWorld->CreateEntity(Archetype::Create<CTransform, CStaticMesh>());
    
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
