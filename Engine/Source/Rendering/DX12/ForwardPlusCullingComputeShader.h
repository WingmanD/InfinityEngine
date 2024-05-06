#pragma once

#include "DX12ComputeShader.h"
#include "StructuredBuffer.h"
#include "Rendering/ForwardPlusCore.h"
#include "Rendering/PointLight.h"
#include "Containers/DynamicGPUBuffer2.h"
#include "ForwardPlusCullingComputeShader.reflection.h"

REFLECTED()
class ForwardPlusCullingComputeShader : public DX12ComputeShader
{
    GENERATED()
    
public:
    ForwardPlusCullingComputeShader();
    
    ForwardPlusCullingComputeShader(const ForwardPlusCullingComputeShader& other);
    ForwardPlusCullingComputeShader& operator=(const ForwardPlusCullingComputeShader& other);

    void InitializeBuffers(DX12GraphicsCommandList& commandList, const D3D12_VIEWPORT& viewport);
    
    void Run(DX12GraphicsCommandList& commandList, const RWStructuredBuffer<SimpleFrustum>& frustumBuffer,
        const DynamicGPUBuffer2<PointLight>& lightsBuffer, const D3D12_VIEWPORT& viewport);
    void Clear(DX12GraphicsCommandList& commandList);

    const RWStructuredBuffer<Tile>& GetTileBuffer() const;
    const RWStructuredBuffer<uint32>& GetIndexBuffer() const;

    // DX12ComputeShader
public:
    virtual bool Initialize() override;
    
private:
    virtual void BindResources(DX12GraphicsCommandList& commandList) const override;

private:
    RWStructuredBuffer<uint32> _counterBuffer{};
    bool _counterBufferInitialized = false;
    
    RWStructuredBuffer<Tile> _tileBuffer{};
    RWStructuredBuffer<uint32> _indexBuffer{};
};
