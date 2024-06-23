#pragma once

#include "System.h"
#include "ECS/Components/CSpawner.h"
#include "ECS/Components/CTeamMember.h"
#include "SpawnerSystem.reflection.h"

REFLECTED()
class SpawnerSystem : public System<CSpawner, const CTeamMember>
{
    GENERATED()
    
public:
    SpawnerSystem() = default;
    SpawnerSystem(const SpawnerSystem& other);

    // System
protected:
    virtual void OnEntityCreated(const Archetype& archetype, Entity& entity) override;
    virtual void ProcessEntityList(EntityList& entityList, double deltaTime) override;
};
