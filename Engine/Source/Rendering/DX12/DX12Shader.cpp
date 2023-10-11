#include "DX12Shader.h"

#include <d3dcompiler.h>
#include <filesystem>
#include "Core.h"
#include "d3dx12.h"
#include "DX12RenderingSubsystem.h"

std::vector<D3D12_INPUT_ELEMENT_DESC> DX12Shader::_inputLayout = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
};

DX12Shader::DX12Shader(std::string name, const std::filesystem::path& importPath) : Shader(std::move(name), importPath)
{
    _rootSignatureDesc = {};
}

void DX12Shader::Apply(ID3D12GraphicsCommandList* commandList)
{
    commandList->SetPipelineState(_pso.Get());
    commandList->SetGraphicsRootSignature(_rootSignature.Get());
}

std::shared_ptr<DX12Shader> DX12Shader::Import(RenderingSubsystem* renderingSubsystem, const std::filesystem::path& path)
{
    DX12RenderingSubsystem* dx12RenderingSubsystem = dynamic_cast<DX12RenderingSubsystem*>(renderingSubsystem);
    if (dx12RenderingSubsystem == nullptr)
    {
        LOG(L"DX12Shader::Import: Invalid rendering subsystem!");
        return nullptr;
    }

    if (path.empty())
    {
        return nullptr;
    }

    if (!exists(path))
    {
        return nullptr;
    }

    // todo read shader model from config
    ComPtr<ID3DBlob> vertexShader = CompileShader(path, nullptr, "VS", "vs_5_1");
    if (vertexShader == nullptr)
    {
        return nullptr;
    }

    ComPtr<ID3DBlob> pixelShader = CompileShader(path, nullptr, "PS", "ps_5_1");
    if (pixelShader == nullptr)
    {
        return nullptr;
    }
    
    std::shared_ptr<DX12Shader> shader = std::make_shared<DX12Shader>(path.stem().string(), path);
    shader->_vertexShader = vertexShader;
    shader->_pixelShader = pixelShader;

    // todo the rest needs to be in Initialize function, we don't want to serialize that
    // todo this should be virtual or parametrized
    CD3DX12_ROOT_PARAMETER slotRootParameter[1];
    
    CD3DX12_DESCRIPTOR_RANGE cbvTable;
    cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);
    
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr, 
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if(errorBlob != nullptr)
    {
        OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(dx12RenderingSubsystem->GetDevice()->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&shader->_rootSignature)));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = {_inputLayout.data(), static_cast<uint32>(_inputLayout.size())};
    psoDesc.pRootSignature = shader->_rootSignature.Get();
    psoDesc.VS =
    {
        static_cast<BYTE*>(shader->_vertexShader->GetBufferPointer()),
        shader->_vertexShader->GetBufferSize()
    };
    psoDesc.PS =
    {
        static_cast<BYTE*>(shader->_pixelShader->GetBufferPointer()),
        shader->_pixelShader->GetBufferSize()
    };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = dx12RenderingSubsystem->GetFrameBufferFormat();
    psoDesc.SampleDesc.Count = dx12RenderingSubsystem->GetMSAASampleCount();
    psoDesc.SampleDesc.Quality = dx12RenderingSubsystem->GetMSAAQuality() - 1;
    psoDesc.DSVFormat = dx12RenderingSubsystem->GetDepthStencilFormat();
    ThrowIfFailed(dx12RenderingSubsystem->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&shader->_pso)));
    
    return shader;
}

bool DX12Shader::Serialize(MemoryWriter& writer)
{
    if (!Shader::Serialize(writer))
    {
        return false;
    }

    // todo root signature, shaders
    return true;
}

bool DX12Shader::Deserialize(MemoryReader& reader)
{
    if (!Shader::Deserialize(reader))
    {
        return false;
    }

    // todo
    return true;
}

const D3D12_ROOT_SIGNATURE_DESC& DX12Shader::GetRootSignatureDesc(PassKey<DX12RenderingSubsystem>) const
{
    return _rootSignatureDesc;
}

ComPtr<ID3DBlob> DX12Shader::CompileShader(const std::filesystem::path& shaderPath,
                                           const D3D_SHADER_MACRO* defines,
                                           const std::string& entryPoint,
                                           const std::string& target)
{
    uint32 compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> byteCode = nullptr;
    ComPtr<ID3DBlob> errors;

    const HRESULT hr = D3DCompileFromFile(shaderPath.c_str(),
                                          defines,
                                          D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                          entryPoint.c_str(),
                                          target.c_str(),
                                          compileFlags,
                                          0,
                                          &byteCode,
                                          &errors);

    if (errors != nullptr)
    {
        OutputDebugStringA(static_cast<char*>(errors->GetBufferPointer()));
    }

    if (FAILED(hr))
    {
        LOG(L"Shader compilation failed: {}", shaderPath.wstring());

        return nullptr;
    }

    return byteCode;
}
