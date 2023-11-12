#include "DX12StaticMeshRenderingData.h"
#include <d3dcompiler.h>
#include "Rendering/StaticMesh.h"
#include "DX12RenderingSubsystem.h"
#include "DX12MaterialRenderingData.h"
#include <vector>

void DX12StaticMeshRenderingData::SetupDrawing(ID3D12GraphicsCommandList* commandList) const
{
    GetMesh()->GetMaterial()->GetRenderingData<DX12MaterialRenderingData>()->Apply(commandList);

    commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
    commandList->IASetIndexBuffer(&_indexBufferView);

    // todo temporary
    commandList->DrawIndexedInstanced(GetMeshRaw().GetIndices().size(), 1, 0, 0, 0);
}

bool DX12StaticMeshRenderingData::UploadToGPUInternal(RenderingSubsystem& renderingSubsystem)
{
    DX12RenderingSubsystem& dx12RenderingSubsystem = dynamic_cast<DX12RenderingSubsystem&>(renderingSubsystem);

    std::shared_ptr<StaticMesh> mesh = GetMesh();

    const std::vector<Vertex>& vertices = mesh->GetVertices();
    const uint32 vertexBufferByteSize = vertices.size() * sizeof(Vertex);
    HRESULT hr = D3DCreateBlob(vertexBufferByteSize, &_vertexBufferCpu);
    if (FAILED(hr))
    {
        LOG(L"Failed to create vertex buffer blob");
        return false;
    }
    CopyMemory(_vertexBufferCpu->GetBufferPointer(), vertices.data(), vertexBufferByteSize);

    const std::vector<uint32_t>& indices = mesh->GetIndices();
    const uint32 indexBufferByteSize = indices.size() * sizeof(uint32_t);
    hr = D3DCreateBlob(indexBufferByteSize, &_indexBufferCpu);
    if (FAILED(hr))
    {
        LOG(L"Failed to create index buffer blob");
        return false;
    }
    CopyMemory(_indexBufferCpu->GetBufferPointer(), indices.data(), indexBufferByteSize);

    DX12CopyCommandList commandList = dx12RenderingSubsystem.RequestCopyCommandList();

    _vertexBufferGpu = dx12RenderingSubsystem.CreateDefaultBuffer(commandList.CommandList.Get(), vertices.data(), vertexBufferByteSize, _vertexBufferUploader);
    if (_vertexBufferGpu == nullptr)
    {
        LOG(L"Failed to create vertex buffer");
        return false;
    }

    _indexBufferGpu = dx12RenderingSubsystem.CreateDefaultBuffer(commandList.CommandList.Get(), indices.data(), indexBufferByteSize, _indexBufferUploader);
    if (_indexBufferGpu == nullptr)
    {
        LOG(L"Failed to create index buffer");
        return false;
    }

    std::weak_ptr weakMesh = mesh;
    commandList.OnCompletedCallbacks.push_back([weakMesh]()
    {
        const std::shared_ptr<StaticMesh> sharedMesh = weakMesh.lock();
        if (sharedMesh == nullptr)
        {
            return;
        }

        sharedMesh->GetRenderingData()->PostUpload();
    });

    dx12RenderingSubsystem.ReturnCopyCommandList(commandList);

    _vertexBufferView.BufferLocation = _vertexBufferGpu->GetGPUVirtualAddress();
    _vertexBufferView.StrideInBytes = sizeof(Vertex);
    _vertexBufferView.SizeInBytes = vertexBufferByteSize;

    _indexBufferView.BufferLocation = _indexBufferGpu->GetGPUVirtualAddress();

    if (indices.size() > std::numeric_limits<int16>::max())
    {
        _indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    }
    else
    {
        _indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    }
    _indexBufferView.SizeInBytes = indexBufferByteSize;

    return true;
}

void DX12StaticMeshRenderingData::PostUpload()
{
    StaticMeshRenderingData::PostUpload();

    _vertexBufferUploader = nullptr;
    _indexBufferUploader = nullptr;
}
