#include "DX12StaticMeshRenderingData.h"
#include "Rendering/StaticMesh.h"
#include "DX12RenderingSubsystem.h"
#include "DX12MaterialRenderingData.h"
#include <vector>

void DX12StaticMeshRenderingData::SetupDrawing(DX12GraphicsCommandList* commandList, const std::shared_ptr<Material>& material) const
{
    material->GetRenderingData<DX12MaterialRenderingData>()->Apply(commandList);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
    commandList->IASetIndexBuffer(&_indexBufferView);
}

void DX12StaticMeshRenderingData::DrawDirect(DX12GraphicsCommandList* commandList, const std::shared_ptr<Material>& material) const
{
    SetupDrawing(commandList, material);
    
    commandList->DrawIndexedInstanced(static_cast<uint32>(GetMeshRaw().GetLOD(0).Indices.Count()), 1, 0, 0, 0);
}

bool DX12StaticMeshRenderingData::UploadToGPUInternal(RenderingSubsystem& renderingSubsystem)
{
    DX12RenderingSubsystem& dx12RenderingSubsystem = dynamic_cast<DX12RenderingSubsystem&>(renderingSubsystem);
    IDxcUtils& dxcUtils = dx12RenderingSubsystem.GetDXCUtils();

    const uint8 lodIndex = GetLOD();
    const std::shared_ptr<StaticMesh> mesh = GetMesh();
    const StaticMesh::LOD& lod = mesh->GetLOD(lodIndex);

    const DArray<Vertex>& vertices = lod.Vertices;
    const uint32 vertexBufferByteSize = static_cast<uint32>(vertices.Count() * sizeof(Vertex));
    HRESULT hr = dxcUtils.CreateBlob(vertices.GetData(), vertexBufferByteSize, DXC_CP_ACP, &_vertexBufferCpu);
    if (FAILED(hr))
    {
        LOG(L"Failed to create vertex buffer blob");
        return false;
    }

    const DArray<uint32_t>& indices = lod.Indices;
    const uint32 indexBufferByteSize = static_cast<uint32>(indices.Count() * sizeof(uint32_t));
    hr = dxcUtils.CreateBlob(indices.GetData(), indexBufferByteSize, DXC_CP_ACP, &_indexBufferCpu);
    if (FAILED(hr))
    {
        LOG(L"Failed to create index buffer blob");
        return false;
    }

    DX12CopyCommandList commandList = dx12RenderingSubsystem.RequestCopyCommandList();

    _vertexBufferGpu = dx12RenderingSubsystem.CreateDefaultBuffer(commandList.CommandList.Get(), vertices.GetData(), vertexBufferByteSize, _vertexBufferUploader);
    if (_vertexBufferGpu == nullptr)
    {
        LOG(L"Failed to create vertex buffer");
        return false;
    }

    _indexBufferGpu = dx12RenderingSubsystem.CreateDefaultBuffer(commandList.CommandList.Get(), indices.GetData(), indexBufferByteSize, _indexBufferUploader);
    if (_indexBufferGpu == nullptr)
    {
        LOG(L"Failed to create index buffer");
        return false;
    }

    std::weak_ptr weakMesh = mesh;
    
    commandList.OnCompletedCallbacks.push_back([weakMesh, lodIndex]()
    {
        const std::shared_ptr<StaticMesh> sharedMesh = weakMesh.lock();
        if (sharedMesh == nullptr)
        {
            return;
        }

        sharedMesh->GetLOD(lodIndex).RenderingData->PostUpload();
    });

    dx12RenderingSubsystem.ReturnCopyCommandList(commandList);

    _vertexBufferView.BufferLocation = _vertexBufferGpu->GetGPUVirtualAddress();
    _vertexBufferView.StrideInBytes = sizeof(Vertex);
    _vertexBufferView.SizeInBytes = static_cast<uint32>(vertexBufferByteSize);

    _indexBufferView.BufferLocation = _indexBufferGpu->GetGPUVirtualAddress();

    if (indices.Count() > std::numeric_limits<int16>::max())
    {
        _indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    }
    else
    {
        _indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    }
    _indexBufferView.SizeInBytes = static_cast<uint32>(indexBufferByteSize);

    return true;
}

void DX12StaticMeshRenderingData::PostUpload()
{
    StaticMeshRenderingData::PostUpload();

    _vertexBufferUploader = nullptr;
    _indexBufferUploader = nullptr;
}
