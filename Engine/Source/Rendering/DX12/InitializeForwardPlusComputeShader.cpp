#include "InitializeForwardPlusComputeShader.h"
#include "DX12MaterialParameterRenderingData.h"
#include "DX12RenderingSubsystem.h"
#include "MaterialParameterTypes.h"

void InitializeForwardPlusComputeShader::Run(
    DX12GraphicsCommandList& commandList,
    const RWStructuredBuffer<SimpleFrustum>& frustumBuffer,
    const D3D12_VIEWPORT& viewport) const
{
    DX12Statics::Transition(
        &commandList,
        frustumBuffer.GetBuffer().Get(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS
    );

    commandList.SetPipelineState(GetPSO());
    commandList.SetComputeRootSignature(GetRootSignature());

    BindResources(commandList);
    commandList.SetComputeRootUnorderedAccessView(1, frustumBuffer.GetUAVGPUVirtualAddress());
    
    const uint32 numX = static_cast<uint32>(Math::Ceil(viewport.Width / Math::Square<float>(ERenderingSettings::ForwardPlusTileDim)));
    const uint32 numY = static_cast<uint32>(Math::Ceil(viewport.Height / Math::Square<float>(ERenderingSettings::ForwardPlusTileDim)));
    commandList.Dispatch(numX, numY, 1);

    DX12Statics::TransitionUAV(commandList, frustumBuffer.GetBuffer().Get());

    DX12Statics::Transition(
        &commandList,
        frustumBuffer.GetBuffer().Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COMMON
    );
}

void InitializeForwardPlusComputeShader::BindResources(DX12GraphicsCommandList& commandList) const
{
    DX12ComputeShader::BindResources(commandList);

    const std::shared_ptr<Scene> scene = RenderingSubsystem::Get().GetScene();
    const ConstantBuffer<MaterialParameter>& constantBuffer =
        static_cast<DX12MaterialParameterRenderingData*>(scene->GetRenderingData())->GetConstantBuffer();
    if (scene->IsDirty())
    {
        constantBuffer.Update();
        scene->ClearDirty();
    }

    commandList.SetComputeRootConstantBufferView(0, constantBuffer.GetGPUVirtualAddress());
}
