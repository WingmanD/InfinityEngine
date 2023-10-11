#pragma once

#include <d3d12.h>

#include "DX12RenderingSubsystem.h"
#include "Rendering/StaticMesh.h"
#include "DX12StaticMesh.reflection.h"

REFLECTED()
class DX12StaticMesh : public StaticMesh
{
    DX12STATICMESH_GENERATED()
    
public:
    enum class StateFlags : uint8
    {
        None = 0,
        Loaded = 1 << 0,
        Uploaded = 1 << 1
    };
    
    DX12StaticMesh() = default;

    explicit DX12StaticMesh(std::string name);
    
    // we need a method that will load mesh on a separate thread
    // when it is loaded, it will enqueue a command to rendering subsystem to upload it to GPU
    // after that is done (wait for fence), we register the mesh in the engine - in the PSO map, and release uploaders
    void Initialize();

    // todo this is temporary
    void Draw(ID3D12GraphicsCommandList* commandList);

    bool IsLoaded() const
    {
        // todo &
        return _stateFlags == StateFlags::Loaded;
    }

    bool IsUploaded() const
    {
        // todo &
        return _stateFlags == StateFlags::Uploaded;
    }
    
private:
    StateFlags _stateFlags = StateFlags::None;
    
    ComPtr<ID3DBlob> _vertexBufferCpu = nullptr;
    ComPtr<ID3DBlob> _indexBufferCpu  = nullptr;

    ComPtr<ID3D12Resource> _vertexBufferGpu = nullptr;
    ComPtr<ID3D12Resource> _indexBufferGpu = nullptr;

    ComPtr<ID3D12Resource> _vertexBufferUploader = nullptr;
    ComPtr<ID3D12Resource> _indexBufferUploader = nullptr;

    D3D12_VERTEX_BUFFER_VIEW _vertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW _indexBufferView = {};

private:
    bool InitializeInternal(DX12RenderingSubsystem& renderingSubsystem);
    void PostInitialize();
};
