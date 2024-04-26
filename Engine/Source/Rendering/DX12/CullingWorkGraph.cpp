﻿#include "CullingWorkGraph.h"

#include "DX12MaterialParameterRenderingData.h"
#include "DX12RenderingSubsystem.h"
#include "MaterialParameterTypes.h"
#include "Engine/Subsystems/AssetManager.h"
#include "Rendering/StaticMesh.h"

void CullingWorkGraph::SetInstanceBuffer(InstanceBuffer* instanceBuffer)
{
    if (_instanceBuffer == instanceBuffer)
    {
        return;
    }
    
    _instanceBuffer = instanceBuffer;

    DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();

    _visibleInstancesBuffer.Initialize(
        _instanceBuffer->Count(),
        *renderingSubsystem.GetDevice(),
        renderingSubsystem.GetCBVHeap()
    );
}

void CullingWorkGraph::ReadBackVisibleInstances(DX12GraphicsCommandList* commandList)
{
    _visibleInstancesBuffer.ReadBack(commandList);
}

const AppendStructuredBuffer<SMInstance>& CullingWorkGraph::GetVisibleInstances() const
{
    return _visibleInstancesBuffer;
}

bool CullingWorkGraph::Initialize()
{
    SetLibraryPath(AssetManager::Get().GetProjectRootPath() / "Engine/Content/Shaders/CullingWorkGraph.hlsl");
    SetName(L"CullingWorkGraph");
    
    if (!WorkGraph::Initialize())
    {
        return false;
    }

    return true;
}

void CullingWorkGraph::PreDispatch(DX12GraphicsCommandList* commandList)
{
    WorkGraph::PreDispatch(commandList);
    
    _visibleInstancesBuffer.ResetCounter(commandList);

    DX12RenderingSubsystem::Get().UpdateDynamicBuffer(StaticMesh::GetMeshInfoBuffer(), commandList);
}

void CullingWorkGraph::BindBuffers(DX12GraphicsCommandList* commandList) const
{
    commandList->SetComputeRootDescriptorTable(0, _visibleInstancesBuffer.GetUAVGPUHandle());

    const std::shared_ptr<Scene> scene = RenderingSubsystem::Get().GetScene();
    DX12MaterialParameterRenderingData* renderingData = static_cast<DX12MaterialParameterRenderingData*>(scene->GetRenderingData());
    const ConstantBuffer<MaterialParameter>& constantBuffer = renderingData->GetConstantBuffer();
    if (scene->IsDirty())
    {
        constantBuffer.Update();
        scene->ClearDirty();
    }
    
    commandList->SetComputeRootConstantBufferView(1, constantBuffer.GetGPUVirtualAddress());
    commandList->SetComputeRootShaderResourceView(2, StaticMesh::GetMeshInfoBuffer().GetBuffer<DX12GPUBuffer>().GetSRVGPUVirtualAddress());
}

void CullingWorkGraph::DispatchImplementation(DX12GraphicsCommandList* commandList, SMInstance* data, uint32 count)
{
    // UINT recSizeInBytes = WG.spWGProps->GetEntrypointRecordSizeInBytes(0, 0);
    // UINT recSizeInUints = (recSizeInBytes + 3) / 4;
    // D3D12_NODE_GPU_INPUT GPUInput;
    // GPUInput.EntrypointIndex = 0;
    // GPUInput.NumRecords = inputs[0].NumRecords;
    // GPUInput.Records = { inputDataFromGPU[0]->GetGPUVirtualAddress(),recSizeInUints * 4 };
    // MakeBufferAndInitialize(D3D, &rootInputDescFromGPU, &GPUInput, sizeof(GPUInput),
    //                         &pStagingResource2, false);
    //
    // D3D12_DISPATCH_GRAPH_DESC desc = {};
    // desc.Mode = D3D12_DISPATCH_MODE_NODE_GPU_INPUT;
    // desc.NodeGPUInput = rootInputDescFromGPU->GetGPUVirtualAddress();
    // commandList->DispatchGraph(&desc);

    D3D12_DISPATCH_GRAPH_DESC desc = {};
    desc.Mode = D3D12_DISPATCH_MODE_NODE_CPU_INPUT;
    desc.NodeCPUInput.EntrypointIndex = 0;
    desc.NodeCPUInput.NumRecords = count;
    desc.NodeCPUInput.RecordStrideInBytes = sizeof(SMInstance);
    desc.NodeCPUInput.pRecords = data;
    commandList->DispatchGraph(&desc);
}
