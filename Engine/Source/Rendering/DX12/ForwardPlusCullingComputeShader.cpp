#include "Rendering/DX12/ForwardPlusCullingComputeShader.h"

#include "DX12RenderingSubsystem.h"
#include "Rendering/DX12/DX12MaterialParameterRenderingData.h"
#include "Rendering/DX12/ConstantBuffer.h"
#include "MaterialParameterTypes.h"
#include "Engine/Subsystems/RenderingSubsystem.h"

ForwardPlusCullingComputeShader::ForwardPlusCullingComputeShader()
{
}

ForwardPlusCullingComputeShader::ForwardPlusCullingComputeShader(
    const ForwardPlusCullingComputeShader& other) : DX12ComputeShader(other)
{
}

ForwardPlusCullingComputeShader& ForwardPlusCullingComputeShader::operator=(
    const ForwardPlusCullingComputeShader& other)
{
    if (this == &other)
    {
        return *this;
    }

    return *this;
}

void ForwardPlusCullingComputeShader::InitializeBuffers(DX12GraphicsCommandList& commandList,
    const D3D12_VIEWPORT& viewport)
{
    const uint32 numX = static_cast<uint32>(Math::Ceil(
        viewport.Width / Math::Square<float>(ERenderingSettings::ForwardPlusTileDim)));
    const uint32 numY = static_cast<uint32>(Math::Ceil(
        viewport.Height / Math::Square<float>(ERenderingSettings::ForwardPlusTileDim)));

    DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();
    const uint32 numTiles = numX * numY;
    if (static_cast<uint32>(_tileBuffer.GetData().Capacity()) != numTiles)
    {
        _tileBuffer.~RWStructuredBuffer();
        new(&_tileBuffer) RWStructuredBuffer<Tile>();

        _tileBuffer.Initialize(numTiles, *renderingSubsystem.GetDevice(), renderingSubsystem.GetCBVHeap());
        _tileBuffer.GetBuffer()->SetName(L"ForwardPlusTileBuffer");
        DX12Statics::Transition(
            &commandList,
            _tileBuffer.GetBuffer().Get(),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        );

        // todo we should have an InitializeResources method for stuff like this
        if (!_counterBufferInitialized)
        {
            DX12Statics::Transition(
                &commandList,
                _counterBuffer.GetBuffer().Get(),
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS
            );
            _counterBufferInitialized = true;
        }
    }

    if (_indexBuffer.GetData().Capacity() < numTiles * ForwardPlusTileMaxLights)
    {
        _indexBuffer.~RWStructuredBuffer();
        new(&_indexBuffer) RWStructuredBuffer<uint32>();

        _indexBuffer.Initialize(numTiles * ForwardPlusTileMaxLights, *renderingSubsystem.GetDevice(),
                                renderingSubsystem.GetCBVHeap());
        _indexBuffer.GetBuffer()->SetName(L"ForwardPlusIndexBuffer");
        DX12Statics::Transition(
            &commandList,
            _indexBuffer.GetBuffer().Get(),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        );
    }
}

void ForwardPlusCullingComputeShader::Run(
    DX12GraphicsCommandList& commandList,
    const RWStructuredBuffer<SimpleFrustum>& frustumBuffer,
    const DynamicGPUBuffer2<PointLight>& lightsBuffer,
    const D3D12_VIEWPORT& viewport)
{
    DX12Statics::Transition(
        &commandList,
        _tileBuffer.GetBuffer().Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS
    );

    DX12Statics::Transition(
        &commandList,
        _indexBuffer.GetBuffer().Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS
    );

    commandList.SetPipelineState(GetPSO());
    commandList.SetComputeRootSignature(GetRootSignature());

    BindResources(commandList);
    commandList.SetComputeRoot32BitConstant(1, static_cast<uint32>(lightsBuffer.Count()), 0);
    commandList.SetComputeRootShaderResourceView(2, frustumBuffer.GetSRVGPUVirtualAddress());
    commandList.SetComputeRootShaderResourceView(3, lightsBuffer.GetBuffer<DX12GPUBuffer>().GetSRVGPUVirtualAddress());

    const uint32 numX = static_cast<uint32>(Math::Ceil(
    viewport.Width / Math::Square<float>(ERenderingSettings::ForwardPlusTileDim)));
    const uint32 numY = static_cast<uint32>(Math::Ceil(
        viewport.Height / Math::Square<float>(ERenderingSettings::ForwardPlusTileDim)));
    commandList.Dispatch(numX, numY, 1);

    DX12Statics::TransitionUAV(commandList, _counterBuffer.GetBuffer().Get());
    DX12Statics::TransitionUAV(commandList, _tileBuffer.GetBuffer().Get());
    DX12Statics::TransitionUAV(commandList, _indexBuffer.GetBuffer().Get());
}

void ForwardPlusCullingComputeShader::Clear(DX12GraphicsCommandList& commandList)
{
    DX12Statics::Transition(
        &commandList,
        _counterBuffer.GetBuffer().Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COMMON
    );
    _counterBuffer.Update(&commandList);
    DX12Statics::Transition(
        &commandList,
        _counterBuffer.GetBuffer().Get(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS
    );

    DX12Statics::Transition(
        &commandList,
        _tileBuffer.GetBuffer().Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_COMMON
    );
    _tileBuffer.Update(&commandList);
    DX12Statics::Transition(
        &commandList,
        _tileBuffer.GetBuffer().Get(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );

    DX12Statics::Transition(
        &commandList,
        _indexBuffer.GetBuffer().Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_COMMON
    );
    _indexBuffer.Update(&commandList);
    DX12Statics::Transition(
        &commandList,
        _indexBuffer.GetBuffer().Get(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
}

const RWStructuredBuffer<Tile>& ForwardPlusCullingComputeShader::GetTileBuffer() const
{
    return _tileBuffer;
}

const RWStructuredBuffer<uint32>& ForwardPlusCullingComputeShader::GetIndexBuffer() const
{
    return _indexBuffer;
}

bool ForwardPlusCullingComputeShader::Initialize()
{
    if (!DX12ComputeShader::Initialize())
    {
        return false;
    }

    DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();

    if (!_counterBuffer.Initialize(1, *renderingSubsystem.GetDevice(), renderingSubsystem.GetCBVHeap()))
    {
        return false;
    }

    if (!_tileBuffer.Initialize(1, *renderingSubsystem.GetDevice(), renderingSubsystem.GetCBVHeap()))
    {
        return false;
    }
    _tileBuffer.GetBuffer()->SetName(L"ForwardPlusTileBuffer_Init");

    if (!_indexBuffer.Initialize(1, *renderingSubsystem.GetDevice(), renderingSubsystem.GetCBVHeap()))
    {
        return false;
    }
    _indexBuffer.GetBuffer()->SetName(L"ForwardPlusIndexBuffer_Init");

    return true;
}

void ForwardPlusCullingComputeShader::BindResources(DX12GraphicsCommandList& commandList) const
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
    commandList.SetComputeRootUnorderedAccessView(4, _counterBuffer.GetUAVGPUVirtualAddress());
    commandList.SetComputeRootUnorderedAccessView(5, _tileBuffer.GetUAVGPUVirtualAddress());
    commandList.SetComputeRootUnorderedAccessView(6, _indexBuffer.GetUAVGPUVirtualAddress());
}
