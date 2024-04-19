#pragma once

#include "System.h"
#include "ECS/Components/CStaticMesh.h"
#include "ECS/Components/CTransform.h"
#include "Rendering/InstanceBuffer.h"

struct MaterialParameter;

class StaticMeshRenderingSystem : public System<const CTransform, const CStaticMesh>
{
public:
    InstanceBuffer& GetInstanceBuffer();
    DynamicGPUBuffer<MaterialParameter>& GetMaterialParameterBuffer(uint32 materialID);

    // todo setters for mesh and material to be called from other systems as events

protected:
    virtual void Initialize() override;

    virtual void OnEntityCreated(const Archetype& archetype, Entity& entity) override;

    virtual void Tick(double deltaTime) override;

    virtual void OnEntityDestroyed(const Archetype& archetype, Entity& entity) override;

    virtual void Shutdown() override;

private:
    DArray<CStaticMesh*> _registeredMeshComponents;
    InstanceBuffer _instanceBuffer{};
    std::unordered_map<uint32, DynamicGPUBuffer<MaterialParameter>> _materialIDToMaterialParameterBuffer;

private:
    DynamicGPUBuffer<MaterialParameter>& GetOrCreateMaterialParameterBuffer(uint32 materialID, const std::shared_ptr<Shader>& shader);
};
