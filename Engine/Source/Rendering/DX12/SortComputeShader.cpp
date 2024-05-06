#include "SortComputeShader.h"
#include "DX12RenderingSubsystem.h"

bool SortComputeShader::Recompile(bool immediate)
{
    if (!DX12ComputeShader::Recompile(immediate))
    {
        return false;
    }

    const DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();
    DX12Device* device = renderingSubsystem.GetDevice();
    IDxcUtils& dxcUtils = renderingSubsystem.GetDXCUtils();

    const ComPtr<IDxcResult> compileResult = CompileShader(GetImportPath(), L"MatrixTranspose", L"cs_6_8");
    if (compileResult == nullptr)
    {
        return false;
    }

    ComPtr<ID3DBlob> computeShader = nullptr;
    compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(computeShader.GetAddressOf()), nullptr);
    if (computeShader == nullptr)
    {
        LOG(L"Failed to compile shader {} - failed to get compute shader byte code", GetName());
        return false;
    }

    dxcUtils.CreateBlob(
        computeShader->GetBufferPointer(),
        static_cast<uint32>(computeShader->GetBufferSize()),
        DXC_CP_ACP,
        &_transposeShader
    );

    ShaderReflection reflection;
    if (!ReflectShaderParameters(compileResult.Get(), reflection))
    {
        LOG(L"Failed to compile shader {} - failed to reflect compute shader", GetName());
        return false;
    }

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.NumParameters = static_cast<uint32>(reflection.RootParameters.Count());
    rootSignatureDesc.pParameters = reflection.RootParameters.GetData();
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    ComPtr<ID3DBlob> serializedRootSignature = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
                                             D3D_ROOT_SIGNATURE_VERSION_1,
                                             serializedRootSignature.GetAddressOf(),
                                             errorBlob.GetAddressOf());
    if (errorBlob != nullptr)
    {
        OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
    }

    if (FAILED(hr))
    {
        LOG(L"Failed to serialize root signature for shader {}!", GetName());
        return false;
    }

    hr = renderingSubsystem.GetDevice()->CreateRootSignature(
        0,
        serializedRootSignature->GetBufferPointer(),
        serializedRootSignature->GetBufferSize(),
        IID_PPV_ARGS(&_transposeRootSignature));

    if (FAILED(hr))
    {
        LOG(L"Failed to create root signature for compute shader {}!", GetName());
        return false;
    }

    dxcUtils.CreateBlob(
        serializedRootSignature->GetBufferPointer(),
        static_cast<uint32>(serializedRootSignature->GetBufferSize()),
        DXC_CP_ACP,
        &_serializedTransposeRootSignature
    );

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = _transposeRootSignature.Get();
    psoDesc.CS = CD3DX12_SHADER_BYTECODE(computeShader.Get());

    hr = device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&_transposePSO));

    if (FAILED(hr))
    {
        LOG(L"Failed to create transpose PSO for compute shader {}!", GetName());
        return false;
    }

    return true;
}

SortComputeShader::SortComputeShader(const SortComputeShader& other) : DX12ComputeShader(other)
{
}

SortComputeShader& SortComputeShader::operator=(const SortComputeShader& other)
{
    return *this;
}

void SortComputeShader::Run(DX12GraphicsCommandList& commandList, AppendStructuredBuffer<SMInstance>& instanceBuffer)
{
    const uint32 count = instanceBuffer.Count();
    if (count == 0)
    {
        return;
    }
    
    const uint32 numElements = Math::NextPowerOfTwo(std::max(count, 128u * 16u));
    if (count > _intermediateBuffer.GetData().Capacity())
    {
        DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();

        _intermediateBuffer.~RWStructuredBuffer();
        new(&_intermediateBuffer) RWStructuredBuffer<SMInstance>();

        _intermediateBuffer.Initialize(numElements, *renderingSubsystem.GetDevice(), renderingSubsystem.GetCBVHeap());
    }

    DX12Statics::Transition(&commandList,  _intermediateBuffer.GetBuffer().Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    const uint32 matrixHeight = numElements / _blockSize;
    const uint32 threadCount = static_cast<uint32>(ceil(static_cast<float>(numElements) / static_cast<float>(_blockSize)));

    commandList.SetPipelineState(GetPSO());
    commandList.SetComputeRootSignature(GetRootSignature());
    commandList.SetComputeRootUnorderedAccessView(1, instanceBuffer.GetUAVGPUVirtualAddress());

    for (UINT level = 2; level <= _blockSize; level = level * 2)
    {
        _constants.Level = level;
        _constants.LevelMask = level;
        _constants.Width = matrixHeight;
        _constants.Height = _matrixWidth;

        commandList.SetComputeRoot32BitConstants(0, sizeof(_constants) / 4, &_constants, 0);
        commandList.Dispatch(threadCount, 1, 1);
        DX12Statics::TransitionUAV(commandList, instanceBuffer.GetBuffer().Get());
    }

    for (UINT level = (_blockSize * 2); level <= numElements; level = level * 2)
    {
        _constants.Level = level / _blockSize;
        _constants.LevelMask = (level & ~numElements) / _blockSize;
        _constants.Width = _matrixWidth;
        _constants.Height = matrixHeight;

        commandList.SetPipelineState(_transposePSO.Get());
        commandList.SetComputeRootSignature(_transposeRootSignature.Get());
        commandList.SetComputeRoot32BitConstants(0, sizeof(_constants) / 4, &_constants, 0);
        commandList.SetComputeRootShaderResourceView(1, instanceBuffer.GetUAVGPUVirtualAddress());
        commandList.SetComputeRootUnorderedAccessView(2, _intermediateBuffer.GetUAVGPUVirtualAddress());
        commandList.Dispatch(_matrixWidth / _transposeBlockSize, (matrixHeight) / _transposeBlockSize, 1);
        DX12Statics::TransitionUAV(commandList, _intermediateBuffer.GetBuffer().Get());
        
        commandList.SetPipelineState(GetPSO());
        commandList.SetComputeRootSignature(GetRootSignature());
        commandList.SetComputeRoot32BitConstants(0, sizeof(_constants) / 4, &_constants, 0);
        commandList.SetComputeRootUnorderedAccessView(1, instanceBuffer.GetUAVGPUVirtualAddress());
        commandList.Dispatch(threadCount, 1, 1);
        DX12Statics::TransitionUAV(commandList, instanceBuffer.GetBuffer().Get());

        _constants.Level = _blockSize;
        _constants.LevelMask = level;
        _constants.Width = matrixHeight;
        _constants.Height = _matrixWidth;

        commandList.SetPipelineState(_transposePSO.Get());
        commandList.SetComputeRootSignature(_transposeRootSignature.Get());
        commandList.SetComputeRoot32BitConstants(0, sizeof(_constants) / 4, &_constants, 0);
        commandList.SetComputeRootShaderResourceView(1, _intermediateBuffer.GetUAVGPUVirtualAddress());
        commandList.SetComputeRootUnorderedAccessView(2, instanceBuffer.GetUAVGPUVirtualAddress());
        commandList.Dispatch(matrixHeight / _transposeBlockSize, _matrixWidth / _transposeBlockSize, 1);
        DX12Statics::TransitionUAV(commandList, instanceBuffer.GetBuffer().Get());
        
        commandList.SetPipelineState(GetPSO());
        commandList.SetComputeRootSignature(GetRootSignature());
        commandList.SetComputeRoot32BitConstants(0, sizeof(_constants) / 4, &_constants, 0);
        commandList.SetComputeRootUnorderedAccessView(1, instanceBuffer.GetUAVGPUVirtualAddress());
        commandList.Dispatch(threadCount, 1, 1);
        DX12Statics::TransitionUAV(commandList, instanceBuffer.GetBuffer().Get());
    }

    DX12Statics::Transition(&commandList,  _intermediateBuffer.GetBuffer().Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
}
