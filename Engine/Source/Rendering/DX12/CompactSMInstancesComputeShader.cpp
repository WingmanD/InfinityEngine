#include "Rendering/DX12/CompactSMInstancesComputeShader.h"
#include "Math/Math.h"
#include "Rendering/DX12/DX12RenderingSubsystem.h"

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
    
    const uint32 threads = Math::Ceil(static_cast<float>(instanceCount) / Math::Square(32.0f));
    Dispatch(commandList, threads, 1, 1);
    
    DX12Statics::TransitionUAV(commandList, instanceBuffer.GetBuffer().Get());
}

void CompactSMInstancesComputeShader::BindResources(DX12GraphicsCommandList& commandList) const
{
    DX12ComputeShader::BindResources(commandList);

    commandList.SetComputeRoot32BitConstants(0, sizeof(_rootConstants) / 4, &_rootConstants, 0);
    commandList.SetComputeRootUnorderedAccessView(1, _instanceBuffer->GetUAVGPUVirtualAddress());
}
