#pragma once

#include "System.h"
#include "ECS/Components/CTargeting.h"
#include "ECS/Components/CTeamMember.h"
#include "ECS/Components/CPathfinding.h"
#include "ECS/Components/CTransform.h"
#include "TargetingSystem.reflection.h"

REFLECTED()
class TargetingSystem : public System<const CTransform, CPathfinding, CTargeting, const CTeamMember>
{
    GENERATED()

    // System
public:
    virtual void OnEntityCreated(const Archetype& archetype, Entity& entity) override;
    virtual void ProcessEntityList(EntityList& entityList, double deltaTime) override;
};
