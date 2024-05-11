#pragma once

#include "System.h"
#include "ECS/Components/CPathfinding.h"
#include "ECS/Components/CTransform.h"

class PathfindingSystem : public System<CTransform, CPathfinding>
{
protected:
    virtual void OnEntityCreated(const Archetype& archetype, Entity& entity) override;
    virtual void ProcessEntityList(EntityList& entityList, double deltaTime) override;
};
