#pragma once

#include "DX12ComputeShader.h"
#include "StructuredBuffer.h"
#include "Rendering/InstanceBuffer.h"
#include "CompactSMInstancesComputeShader.reflection.h"

REFLECTED()
class CompactSMInstancesComputeShader : public DX12ComputeShader
{
    GENERATED()
    
public:
    CompactSMInstancesComputeShader() = default;

    CompactSMInstancesComputeShader(const CompactSMInstancesComputeShader& other);
    CompactSMInstancesComputeShader& operator=(const CompactSMInstancesComputeShader& other);

    void Run(DX12GraphicsCommandList& commandList, AppendStructuredBuffer<SMInstance>& instanceBuffer);

    // DX12ComputeShader
protected:
    virtual void BindResources(DX12GraphicsCommandList& commandList) const override;
    
private:
    AppendStructuredBuffer<SMInstance>* _instanceBuffer = nullptr;

    struct RootConstants
    {
        uint32 InstanceCount;
        uint32 InstanceCapacity;
    };

    RootConstants _rootConstants{};
};
