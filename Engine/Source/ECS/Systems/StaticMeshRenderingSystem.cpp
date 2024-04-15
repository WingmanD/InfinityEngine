#include "StaticMeshRenderingSystem.h"
#include "ECS/Entity.h"
#include "ECS/EntityList.h"
#include "Engine/Subsystems/RenderingSubsystem.h"

InstanceBuffer& StaticMeshRenderingSystem::GetInstanceBuffer()
{
    return _instanceBuffer;
}

void StaticMeshRenderingSystem::Initialize()
{
    System::Initialize();

    RenderingSubsystem::Get().RegisterStaticMeshRenderingSystem(this);
}

void StaticMeshRenderingSystem::OnEntityCreated(const Archetype& archetype, Entity& entity)
{
    CTransform& transform = entity.Get<CTransform>(archetype);
    CStaticMesh& staticMesh = entity.Get<CStaticMesh>(archetype);

    staticMesh.Mesh->Load();
    staticMesh.MeshTransform.SetParent(&transform.ComponentTransform);
    if (staticMesh.MaterialOverride == nullptr)
    {
        staticMesh.MaterialOverride = staticMesh.Mesh->GetMaterial();
    }

    SMInstance instanceData;
    instanceData.World = staticMesh.MeshTransform.GetWorldMatrix().Transpose();
    instanceData.MeshID = staticMesh.Mesh->GetAssetID();
    instanceData.MaterialID = staticMesh.MaterialOverride->GetAssetID();

    _instanceBuffer.Reserve(_instanceBuffer.Count() + 1);
    const size_t instanceID = _instanceBuffer.Emplace(std::move(instanceData));
    staticMesh.InstanceID = instanceID;

    _registeredMeshComponents.Add(&staticMesh);
}

void StaticMeshRenderingSystem::Tick(double deltaTime)
{
    for (EntityList* entityList : GetQuery().GetEntityLists())
    {
        const Archetype& archetype = entityList->GetArchetype();
        entityList->ForEach([&archetype, this](Entity& entity)
        {
            const CStaticMesh& staticMesh = entity.Get<CStaticMesh>(archetype);

            // todo optimize, this should be an event
            _instanceBuffer[staticMesh.InstanceID].World = staticMesh.MeshTransform.GetWorldMatrix().Transpose();
            
            return true;
        });
    }
}

void StaticMeshRenderingSystem::OnEntityDestroyed(const Archetype& archetype, Entity& entity)
{
    const CStaticMesh& staticMesh = entity.Get<CStaticMesh>(archetype);

    _instanceBuffer.RemoveAtSwap(staticMesh.InstanceID);
    _instanceBuffer[staticMesh.InstanceID].MeshID = _registeredMeshComponents.Back()->InstanceID;

    _registeredMeshComponents.RemoveAtSwap(staticMesh.InstanceID);
}

void StaticMeshRenderingSystem::Shutdown()
{
    System::Shutdown();

    RenderingSubsystem::Get().UnregisterStaticMeshRenderingSystem(this);
}
