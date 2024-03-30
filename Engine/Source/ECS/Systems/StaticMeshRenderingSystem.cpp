#include "StaticMeshRenderingSystem.h"
#include "MaterialParameterTypes.h"
#include "ECS/Entity.h"
#include "ECS/EntityList.h"
#include "Engine/Subsystems/RenderingSubsystem.h"
#include "Rendering/DX12/DX12RenderingSubsystem.h"
#include "Rendering/DX12/DX12StaticMeshRenderingData.h"

void StaticMeshRenderingSystem::OnEntityCreated(const Archetype& archetype, Entity& entity)
{
    CTransform* transform = entity.Get<CTransform>(archetype);
    CStaticMesh* staticMesh = entity.Get<CStaticMesh>(archetype);

    staticMesh->Mesh->Load();
    staticMesh->MeshTransform.SetParent(&transform->ComponentTransform);
    if (staticMesh->MaterialOverride == nullptr)
    {
        staticMesh->MaterialOverride = staticMesh->Mesh->GetMaterial();
    }
}

void StaticMeshRenderingSystem::Tick(double deltaTime)
{
    // todo we shouldn't bother using work graphs - they are currently ideal for deferred shading and we are doing
    // forward+ shading. Main rendering system should have a list of currently visible mesh instances from each camera,
    // sorted by material and mesh. Then we can just iterate over them and draw instanced them.
    // Later, we must implement a BVH for frustum culling (what about occlusion culling? how does it fit here?)
    
    // DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();
    // DX12CommandList commandList = renderingSubsystem.RequestCommandList();
    //
    // for (EntityList* entityList : GetQuery().GetEntityLists())
    // {
    //     const Archetype& archetype = entityList->GetArchetype();
    //     entityList->ForEach([&archetype, &commandList](Entity& entity)
    //     {
    //         const CStaticMesh* staticMesh = entity.Get<CStaticMesh>(archetype);
    //         if (staticMesh->Mesh->GetRenderingData()->IsUploaded())
    //         {
    //             StaticMeshConstants* constants = staticMesh->Mesh->GetMaterial()->GetParameter<StaticMeshConstants>("GStaticMeshConstants");
    //             constants->Transform = staticMesh->MeshTransform.GetWorldMatrix();
    //             
    //             static_cast<DX12StaticMeshRenderingData*>(staticMesh->Mesh->GetRenderingData())->SetupDrawing(
    //                 commandList.CommandList.Get(), staticMesh->Mesh->GetMaterial());
    //         }
    //         return true;
    //     });
    // }
    //
    // renderingSubsystem.CloseCommandList(commandList);
}
