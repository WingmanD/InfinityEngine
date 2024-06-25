#include "ECS/Systems/StaticMeshRenderingSystem.h"
#include "MaterialParameterTypes.h"
#include "ECS/Entity.h"
#include "Engine/Subsystems/RenderingSubsystem.h"

StaticMeshRenderingSystem::StaticMeshRenderingSystem(const StaticMeshRenderingSystem& other) : System(other)
{
}

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

    _onTransformChangedHandle = GetWorld().OnTransformChanged.RegisterListener(_onTransformChanged);
    _onArchetypeChangedHandle = GetWorld().OnArchetypeChanged.RegisterListener(_onArchetypeChanged);
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
    
    // staticMesh.MaterialOverride->GetParameterMap()->GetDefaultParameters()[0].Parameter->DuplicateAt(&materialInstanceBuffer[instanceData.MaterialIndex]);

    DefaultMaterialParameter& materialInstance = static_cast<DefaultMaterialParameter&>(materialInstanceBuffer[instanceData.MaterialIndex]);
    const DefaultMaterialParameter& defaultInstance = *static_cast<DefaultMaterialParameter*>(
        staticMesh.MaterialOverride->GetParameterMap()->GetDefaultParameters()[0].Parameter.get()
    );
    materialInstance.BaseColor = defaultInstance.BaseColor;
    materialInstance.Metallic = defaultInstance.Metallic;

    // SharedObjectPtr<MaterialParameter> param = staticMesh.MaterialOverride->GetParameterMap()->GetDefaultParameters()[0].Parameter;
    // Type* type = param->GetType();
    // const std::byte* byteData = reinterpret_cast<std::byte*>(param.get()) + type->GetDataOffset();
    // memcpy(&materialInstanceBuffer[instanceData.MaterialIndex], byteData, type->GetSize() - type->GetDataOffset());

    _instanceBuffer.Reserve(_instanceBuffer.Count() + 1);
    const uint32 instanceID = _instanceBuffer.Emplace(std::move(instanceData));
    staticMesh.InstanceID = instanceID;

    _registeredMeshComponents.Add(&staticMesh);
}

void StaticMeshRenderingSystem::Tick(double deltaTime)
{
    GetEventQueue().ProcessEvents();
    
    for (auto& entityListStruct : _onArchetypeChanged.GetEntityLists())
    {
        const uint16 staticMeshIndex = entityListStruct.EntityArchetype.GetComponentIndexChecked<CStaticMesh>();
        if (staticMeshIndex == std::numeric_limits<uint16>::max())
        {
            continue;
        }

        EventArchetypeChanged::EventData eventData;
        while (entityListStruct.Queue.Dequeue(eventData))
        {
            CStaticMesh& oldStaticMesh = eventData.Entity->Get<CStaticMesh>(staticMeshIndex);
            CStaticMesh& newStaticMesh = eventData.Entity->Get<CStaticMesh>(std::get<Archetype>(eventData.Arguments));
            _registeredMeshComponents[oldStaticMesh.InstanceID] = &newStaticMesh;
        }
    }
    
    for (Event<TypeSet<CTransform>>::EntityListStruct& entityListStruct : _onTransformChanged.GetEntityLists())
    {
        const uint16 index = entityListStruct.EntityArchetype.GetComponentIndexChecked<CStaticMesh>();
        if (index == std::numeric_limits<uint16>::max())
        {
            continue;
        }

        EventTransformChanged::EventData eventData;
        while (entityListStruct.Queue.Dequeue(eventData))
        {
            if (!eventData.Entity->IsValid() || eventData.Entity->GetID() != eventData.ID)
            {
                continue;
            }

            const CStaticMesh& staticMesh = eventData.Entity->Get<CStaticMesh>(index);
            _instanceBuffer[staticMesh.InstanceID].World = staticMesh.MeshTransform.GetWorldMatrix().Transpose();
        }
    }
     
    GetEventQueue().ProcessEvents();
}

void StaticMeshRenderingSystem::OnEntityDestroyed(const Archetype& archetype, Entity& entity)
{
    const CStaticMesh& staticMesh = entity.Get<CStaticMesh>(archetype);

    _instanceBuffer.RemoveAtSwap(staticMesh.InstanceID);

    _registeredMeshComponents.Back()->InstanceID = staticMesh.InstanceID;

    _registeredMeshComponents.RemoveAtSwap(staticMesh.InstanceID);
}

void StaticMeshRenderingSystem::Shutdown()
{
    System::Shutdown();

    World& world = GetWorld();
    world.OnTransformChanged.UnregisterListener(_onTransformChangedHandle);
    world.OnArchetypeChanged.UnregisterListener(_onArchetypeChangedHandle);
    
    RenderingSubsystem::Get().UnregisterStaticMeshRenderingSystem(this);
}

DynamicGPUBuffer<MaterialParameter>& StaticMeshRenderingSystem::GetOrCreateMaterialParameterBuffer(
    uint32 materialID, const SharedObjectPtr<Shader>& shader)
{
    auto it = _materialIDToMaterialParameterBuffer.find(materialID);
    if (it != _materialIDToMaterialParameterBuffer.end())
    {
        return it->second;
    }

    const auto result = _materialIDToMaterialParameterBuffer.emplace(materialID, DynamicGPUBuffer<MaterialParameter>());
    RenderingSubsystem::Get().InitializeMaterialInstanceBuffer(result.first->second, shader->GetMaterialInstanceDataType());

    return result.first->second;
}
