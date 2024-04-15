#pragma once

#include "System.h"
#include "ECS/Components/CStaticMesh.h"
#include "ECS/Components/CTransform.h"
#include "Rendering/InstanceBuffer.h"

class StaticMeshRenderingSystem : public System<const CTransform, const CStaticMesh>
{
public:
    InstanceBuffer& GetInstanceBuffer();
    
protected:
    virtual void Initialize() override;

    virtual void OnEntityCreated(const Archetype& archetype, Entity& entity) override;

    virtual void Tick(double deltaTime) override;

    virtual void OnEntityDestroyed(const Archetype& archetype, Entity& entity) override;

    virtual void Shutdown() override;

private:
    DArray<CStaticMesh*> _registeredMeshComponents;
    InstanceBuffer _instanceBuffer{};
};
