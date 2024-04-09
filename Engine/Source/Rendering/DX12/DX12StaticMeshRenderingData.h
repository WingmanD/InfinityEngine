#pragma once

#include "Rendering/DX12/DX12RenderingCore.h"
#include "Rendering/StaticMeshRenderingData.h"
#include <d3d12.h>
#include <wrl/client.h>

class Material;
class StaticMesh;
using Microsoft::WRL::ComPtr;

class DX12StaticMeshRenderingData : public StaticMeshRenderingData
{
public:
    DX12StaticMeshRenderingData() = default;

    void SetupDrawing(DX12GraphicsCommandList* commandList, const std::shared_ptr<Material>& material) const;
    void DrawDirect(DX12GraphicsCommandList* commandList, const std::shared_ptr<Material>& material) const;

protected:
    virtual bool UploadToGPUInternal(RenderingSubsystem& renderingSubsystem) override;
    virtual void PostUpload() override;
    
private:
    ComPtr<IDxcBlobEncoding> _vertexBufferCpu = nullptr;
    ComPtr<ID3D12Resource> _vertexBufferGpu = nullptr;
    ComPtr<ID3D12Resource> _vertexBufferUploader = nullptr;
    D3D12_VERTEX_BUFFER_VIEW _vertexBufferView = {};

    ComPtr<IDxcBlobEncoding> _indexBufferCpu = nullptr;
    ComPtr<ID3D12Resource> _indexBufferGpu = nullptr;
    ComPtr<ID3D12Resource> _indexBufferUploader = nullptr;
    D3D12_INDEX_BUFFER_VIEW _indexBufferView = {};
};
