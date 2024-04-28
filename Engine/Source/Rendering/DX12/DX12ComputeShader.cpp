#include "DX12ComputeShader.h"
#include "DX12RenderingSubsystem.h"

bool DX12ComputeShader::Recompile(bool immediate)
{
    const DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();
    DX12Device* device = renderingSubsystem.GetDevice();
    IDxcUtils& dxcUtils = renderingSubsystem.GetDXCUtils();

    const ComPtr<IDxcResult> compileResult = CompileShader(GetImportPath(), L"CS", L"cs_6_8");
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

    std::vector<D3D12_ROOT_PARAMETER> rootParameters;
    std::set<MaterialParameterDescriptor> constantBufferParameterTypes;
    std::set<StructuredBufferParameter> structuredBufferParameterTypes;

    if (!ReflectShaderParameters(compileResult.Get(), rootParameters, constantBufferParameterTypes,
                                 structuredBufferParameterTypes))
    {
        LOG(L"Failed to compile shader {} - failed to reflect compute shader", GetName());
        return false;
    }

    dxcUtils.CreateBlob(
        computeShader->GetBufferPointer(),
        static_cast<uint32>(computeShader->GetBufferSize()),
        DXC_CP_ACP,
        &_computeShader
    );

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = static_cast<uint32>(rootParameters.size());
    rootSignatureDesc.pParameters = rootParameters.data();
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
        IID_PPV_ARGS(&_rootSignature));

    if (FAILED(hr))
    {
        LOG(L"Failed to create root signature for compute shader {}!", GetName());
        return false;
    }

    dxcUtils.CreateBlob(
        serializedRootSignature->GetBufferPointer(),
        static_cast<uint32>(serializedRootSignature->GetBufferSize()),
        DXC_CP_ACP,
        &_serializedRootSignature
    );

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = _rootSignature.Get();
    psoDesc.CS = CD3DX12_SHADER_BYTECODE(computeShader.Get());

    hr = device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&_pso));

    if (FAILED(hr))
    {
        LOG(L"Failed to create PSO for compute shader {}!", GetName());
        return false;
    }

    return true;
}

void DX12ComputeShader::Dispatch(
    DX12GraphicsCommandList& commandList,
    uint32 threadGroupCountX,
    uint32 threadGroupCountY,
    uint32 threadGroupCountZ) const
{
    commandList.SetPipelineState(_pso.Get());
    commandList.SetComputeRootSignature(_rootSignature.Get());

    BindResources(commandList);

    commandList.Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

bool DX12ComputeShader::Initialize()
{
    if (!Shader::Initialize())
    {
        return false;
    }
    
    Recompile();
    
    return true; 
}

bool DX12ComputeShader::Serialize(MemoryWriter& writer) const
{
    if (!Shader::Serialize(writer))
    {
        return false;
    }

    return true;
}

bool DX12ComputeShader::Deserialize(MemoryReader& reader)
{
    if (!Shader::Deserialize(reader))
    {
        return false;
    }

    return true;
}

void DX12ComputeShader::BindResources(DX12GraphicsCommandList& commandList) const
{
}

ID3D12PipelineState* DX12ComputeShader::GetPSO() const
{
    return _pso.Get();
}

ID3D12RootSignature* DX12ComputeShader::GetRootSignature() const
{
    return _rootSignature.Get();
}
