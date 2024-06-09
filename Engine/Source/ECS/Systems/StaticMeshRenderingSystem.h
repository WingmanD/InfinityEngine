#pragma once

#include "System.h"
#include "ECS/Components/CStaticMesh.h"
#include "ECS/Components/CTransform.h"
#include "Rendering/InstanceBuffer.h"
#include "StaticMeshRenderingSystem.reflection.h"

struct MaterialParameter;

REFLECTED()
class StaticMeshRenderingSystem : public System<const CTransform, const CStaticMesh>
{
    GENERATED()
    
public:
    StaticMeshRenderingSystem() = default;
    StaticMeshRenderingSystem(const StaticMeshRenderingSystem& other);
    
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

    PROPERTY()
    Event<TypeSet<CTransform>> _onTransformChanged;
    
    EventHandle _onTransformChangedHandle;
    
private:
    DynamicGPUBuffer<MaterialParameter>& GetOrCreateMaterialParameterBuffer(uint32 materialID, const SharedObjectPtr<Shader>& shader);
};
