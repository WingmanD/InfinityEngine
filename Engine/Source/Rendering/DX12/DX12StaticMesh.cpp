#include "DX12StaticMesh.h"

#include <d3dcompiler.h>

#include "DX12RenderingSubsystem.h"
#include "DX12Shader.h"
#include "Engine/Engine.h"
#include "Engine/Subsystems/AssetManager.h"

DX12StaticMesh::DX12StaticMesh(std::string name) : StaticMesh(std::move(name))
{
}

// todo make this virtual, and AssetManager::NewAsset should call this
void DX12StaticMesh::Initialize()
{
    // todo seems like we don't need to synchronize copy command - we just need to wait for fence after we enqueue it to copy queue
    std::weak_ptr weakThis = std::dynamic_pointer_cast<DX12StaticMesh>(shared_from_this());
    Engine::Get().GetRenderingSubsystem()->GetEventQueue().Enqueue([weakThis](RenderingSubsystem* renderingSubsystem)
    {
        std::shared_ptr<DX12StaticMesh> sharedThis = weakThis.lock();
        if (sharedThis == nullptr)
        {
            return;
        }

        DX12RenderingSubsystem* dx12RenderingSubsystem = dynamic_cast<DX12RenderingSubsystem*>(renderingSubsystem);
        if (dx12RenderingSubsystem == nullptr)
        {
            LOG(L"Rendering subsystem is not DX12");
            return;
        }

        if (!sharedThis->InitializeInternal(*dx12RenderingSubsystem))
        {
            LOG(L"Failed to initialize mesh");
            return;
        }
    });
}

void DX12StaticMesh::Draw(ID3D12GraphicsCommandList* commandList)
{
    commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
    commandList->IASetIndexBuffer(&_indexBufferView);
    commandList->DrawIndexedInstanced(GetIndices().size(), 1, 0, 0, 0);
}

bool DX12StaticMesh::InitializeInternal(DX12RenderingSubsystem& renderingSubsystem)
{
    const std::vector<Vertex>& vertices = GetVertices();
    const uint32 vertexBufferByteSize = vertices.size() * sizeof(Vertex);
    HRESULT hr = D3DCreateBlob(vertexBufferByteSize, &_vertexBufferCpu);
    if (FAILED(hr))
    {
        LOG(L"Failed to create vertex buffer blob");
        return false;
    }
    CopyMemory(_vertexBufferCpu->GetBufferPointer(), vertices.data(), vertexBufferByteSize);

    const std::vector<uint32_t>& indices = GetIndices();
    const uint32 indexBufferByteSize = indices.size() * sizeof(uint32_t);
    hr = D3DCreateBlob(indexBufferByteSize, &_indexBufferCpu);
    if (FAILED(hr))
    {
        LOG(L"Failed to create index buffer blob");
        return false;
    }
    CopyMemory(_indexBufferCpu->GetBufferPointer(), indices.data(), indexBufferByteSize);

    DX12CopyCommandList commandList = renderingSubsystem.RequestCopyCommandList();

    _vertexBufferGpu = renderingSubsystem.CreateDefaultBuffer(commandList.CommandList.Get(), vertices.data(), vertexBufferByteSize, _vertexBufferUploader);
    if (_vertexBufferGpu == nullptr)
    {
        LOG(L"Failed to create vertex buffer");
        return false;
    }

    _indexBufferGpu = renderingSubsystem.CreateDefaultBuffer(commandList.CommandList.Get(), indices.data(), indexBufferByteSize, _indexBufferUploader);
    if (_indexBufferGpu == nullptr)
    {
        LOG(L"Failed to create index buffer");
        return false;
    }
    
    std::weak_ptr weakThis = std::dynamic_pointer_cast<DX12StaticMesh>(shared_from_this());
    commandList.OnCompleted = [weakThis]()
    {
        std::shared_ptr<DX12StaticMesh> sharedThis = weakThis.lock();
        if (sharedThis == nullptr)
        {
            return;
        }

        sharedThis->PostInitialize();
    };
    
    renderingSubsystem.ReturnCopyCommandList(commandList);

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

void DX12StaticMesh::PostInitialize()
{
    _vertexBufferUploader = nullptr;
    _indexBufferUploader = nullptr;

    // todo _stateFlags should be a part of base class, actually, Loaded should be a part of AssetClass
    //_stateFlags = StateFlags::Loaded & StateFlags::Uploaded;
    _stateFlags = StateFlags::Uploaded;
}
