#pragma once

#include "System.h"
#include "ECS/Components/CStaticMesh.h"
#include "ECS/Components/CTransform.h"

class StaticMeshRenderingSystem : public System<const CTransform, const CStaticMesh>
{
public:
protected:
    virtual void OnEntityCreated(const Archetype& archetype, Entity& entity) override;
    
    virtual void Tick(double deltaTime) override;
};
