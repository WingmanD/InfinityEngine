#pragma once

#include "Rendering/StaticMeshRenderingData.h"
#include <d3d12.h>
#include <wrl/client.h>

class StaticMesh;
using Microsoft::WRL::ComPtr;

class DX12StaticMeshRenderingData : public StaticMeshRenderingData
{
public:
    DX12StaticMeshRenderingData() = default;

    void SetupDrawing(ID3D12GraphicsCommandList* commandList) const;

protected:
    virtual bool UploadToGPUInternal(RenderingSubsystem& renderingSubsystem) override;
    virtual void PostUpload() override;
    
private:
    ComPtr<ID3DBlob> _vertexBufferCpu = nullptr;
    ComPtr<ID3D12Resource> _vertexBufferGpu = nullptr;
    ComPtr<ID3D12Resource> _vertexBufferUploader = nullptr;
    D3D12_VERTEX_BUFFER_VIEW _vertexBufferView = {};

    ComPtr<ID3DBlob> _indexBufferCpu = nullptr;
    ComPtr<ID3D12Resource> _indexBufferGpu = nullptr;
    ComPtr<ID3D12Resource> _indexBufferUploader = nullptr;
    D3D12_INDEX_BUFFER_VIEW _indexBufferView = {};
};
