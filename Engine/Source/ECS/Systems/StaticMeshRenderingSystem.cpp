﻿#include "StaticMeshRenderingSystem.h"

#include <DirectXColors.h>

#include "MaterialParameterTypes.h"
#include "ECS/Entity.h"
#include "ECS/EntityList.h"
#include "Engine/Subsystems/RenderingSubsystem.h"

InstanceBuffer& StaticMeshRenderingSystem::GetInstanceBuffer()
{
    return _instanceBuffer;
}

DynamicGPUBuffer<MaterialParameter>& StaticMeshRenderingSystem::GetMaterialParameterBuffer(uint32 materialID)
{
    return _materialIDToMaterialParameterBuffer[materialID];
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
    instanceData.MeshID = staticMesh.Mesh->GetMeshID();
    instanceData.MaterialID = staticMesh.MaterialOverride->GetMaterialID();
    instanceData.LOD = 0;
    instanceData.Count = 1;

    DynamicGPUBuffer<MaterialParameter>& materialInstanceBuffer = GetOrCreateMaterialParameterBuffer(
        instanceData.MaterialID,
        staticMesh.MaterialOverride->GetShader());
    instanceData.MaterialIndex = materialInstanceBuffer.AddDefault();

    // todo temporary
    // todo alignment issue with material parameters
    DefaultMaterialParameter& materialInstance = static_cast<DefaultMaterialParameter&>(materialInstanceBuffer[instanceData.MaterialIndex]);

    _instanceBuffer.Reserve(_instanceBuffer.Count() + 1);
    const uint32 instanceID = _instanceBuffer.Emplace(std::move(instanceData));
    staticMesh.InstanceID = instanceID;

    if (instanceID % 2 == 0)
    {
        materialInstance.BaseColor = DirectX::Colors::Green;
    }
    else
    {
        materialInstance.BaseColor = DirectX::Colors::Red;
    }

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

DynamicGPUBuffer<MaterialParameter>& StaticMeshRenderingSystem::GetOrCreateMaterialParameterBuffer(
    uint32 materialID, const std::shared_ptr<Shader>& shader)
{
    auto it = _materialIDToMaterialParameterBuffer.find(materialID);
    if (it != _materialIDToMaterialParameterBuffer.end())
    {
        return it->second;
    }

    const auto result = _materialIDToMaterialParameterBuffer.emplace(materialID, DynamicGPUBuffer<MaterialParameter>());
    RenderingSubsystem::Get().InitializeMaterialInstanceBuffer(result.first->second,
                                                               shader->GetMaterialInstanceDataType());

    return result.first->second;
}
