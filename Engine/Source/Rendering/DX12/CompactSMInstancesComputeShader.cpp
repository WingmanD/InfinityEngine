#include "CompactSMInstancesComputeShader.h"
#include "DX12RenderingSubsystem.h"

CompactSMInstancesComputeShader::CompactSMInstancesComputeShader(const CompactSMInstancesComputeShader& other) :
    DX12ComputeShader(other)
{
}

CompactSMInstancesComputeShader& CompactSMInstancesComputeShader::operator=(
    const CompactSMInstancesComputeShader& other)
{
    return *this;
}

void CompactSMInstancesComputeShader::Run(DX12GraphicsCommandList& commandList,
                                          AppendStructuredBuffer<SMInstance>& instanceBuffer)
{
    const uint32 instanceCount = instanceBuffer.Count();
    if (instanceCount == 0)
    {
        return;
    }

    _instanceBuffer = &instanceBuffer;
    _rootConstants.InstanceCount = instanceCount;
    _rootConstants.InstanceCapacity = static_cast<uint32>(instanceBuffer.GetData().Capacity());
    
    const uint32 threads = std::max(instanceCount / 32u, 1u);
    Dispatch(commandList, threads, 1, 1);
    
    DX12Statics::TransitionUAV(commandList, instanceBuffer.GetBuffer().Get());
}

void CompactSMInstancesComputeShader::BindResources(DX12GraphicsCommandList& commandList) const
{
    DX12ComputeShader::BindResources(commandList);

    commandList.SetComputeRoot32BitConstants(0, sizeof(_rootConstants) / 4, &_rootConstants, 0);
    commandList.SetComputeRootUnorderedAccessView(1, _instanceBuffer->GetUAVGPUVirtualAddress());
}
