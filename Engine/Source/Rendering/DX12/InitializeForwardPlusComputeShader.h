#pragma once

#include "DX12ComputeShader.h"
#include "StructuredBuffer.h"
#include "Rendering/ForwardPlusCore.h"
#include "InitializeForwardPlusComputeShader.reflection.h"

REFLECTED()
class InitializeForwardPlusComputeShader : public DX12ComputeShader
{
    GENERATED()
    
public:
    void Run(DX12GraphicsCommandList& commandList, const RWStructuredBuffer<SimpleFrustum>& frustumBuffer, const D3D12_VIEWPORT& viewport) const;

protected:
    virtual void BindResources(DX12GraphicsCommandList& commandList) const override;
};
