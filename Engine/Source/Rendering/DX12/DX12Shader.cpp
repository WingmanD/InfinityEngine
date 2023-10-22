#include "DX12Shader.h"
#include <d3dcompiler.h>
#include <filesystem>
#include "Core.h"
#include "d3dx12.h"
#include "DX12RenderingSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/Subsystems/AssetManager.h"

std::vector<D3D12_INPUT_ELEMENT_DESC> DX12Shader::_inputLayout = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"UV", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
};

DX12Shader::DX12Shader(const std::wstring& name) : Shader(name)
{
}

DX12Shader::DX12Shader(const DX12Shader& other) : Shader(other)
{
    // todo
}

DX12Shader& DX12Shader::operator=(const DX12Shader& other)
{
    if (this == &other)
    {
        return *this;
    }

    // todo

    return *this;
}

bool DX12Shader::Initialize()
{
    const DX12RenderingSubsystem* renderingSubsystem = dynamic_cast<DX12RenderingSubsystem*>(Engine::Get().GetRenderingSubsystem());
    if (renderingSubsystem == nullptr)
    {
        DEBUG_BREAK();
        return false;
    }

    if (!InitializeRootSignature(*renderingSubsystem))
    {
        return false;
    }

    InitializePSO(*renderingSubsystem);

    return true;
}

void DX12Shader::Apply(ID3D12GraphicsCommandList* commandList)
{
    commandList->SetPipelineState(_pso.Get());
    commandList->SetGraphicsRootSignature(_rootSignature.Get());
}

std::shared_ptr<DX12Shader> DX12Shader::Import(AssetManager& assetManager, const std::filesystem::path& path)
{
    if (path.empty())
    {
        return nullptr;
    }

    if (!exists(path))
    {
        return nullptr;
    }

    std::shared_ptr<DX12Shader> shader = assetManager.NewAsset<DX12Shader>(path.stem().wstring());
    shader->SetImportPath(path);

    if (!shader->Compile())
    {
        assetManager.DeleteAsset(shader);
        return nullptr;
    }

    shader->MarkDirtyForAutosave();

    return shader;
}

bool DX12Shader::Serialize(MemoryWriter& writer) const
{
    if (!Shader::Serialize(writer))
    {
        return false;
    }

    writer << _lastCompileTime;

    if (_serializedRootSignature != nullptr)
    {
        writer << static_cast<uint64>(_serializedRootSignature->GetBufferSize());
    }
    else
    {
        writer << static_cast<uint64>(0ull);
    }

    if (_vertexShader != nullptr)
    {
        writer << static_cast<uint64>(_vertexShader->GetBufferSize());
    }
    else
    {
        writer << static_cast<uint64>(0ull);
    }

    if (_pixelShader != nullptr)
    {
        writer << static_cast<uint64>(_pixelShader->GetBufferSize());
    }
    else
    {
        writer << static_cast<uint64>(0ull);
    }

    if (_serializedRootSignature != nullptr)
    {
        writer.Write(static_cast<const std::byte*>(_serializedRootSignature->GetBufferPointer()), _serializedRootSignature->GetBufferSize());
    }
    if (_vertexShader != nullptr)
    {
        writer.Write(static_cast<const std::byte*>(_vertexShader->GetBufferPointer()), _vertexShader->GetBufferSize());
    }

    if (_pixelShader != nullptr)
    {
        writer.Write(static_cast<const std::byte*>(_pixelShader->GetBufferPointer()), _pixelShader->GetBufferSize());
    }

    return true;
}

bool DX12Shader::Deserialize(MemoryReader& reader)
{
    if (!Shader::Deserialize(reader))
    {
        return false;
    }

    std::filesystem::file_time_type lastWriteTime;
    reader >> lastWriteTime;

    uint64 serializedRootSignatureSize;
    reader >> serializedRootSignatureSize;

    uint64 vertexShaderSize;
    reader >> vertexShaderSize;

    uint64 pixelShaderSize;
    reader >> pixelShaderSize;

    if (lastWriteTime < last_write_time(GetImportPath()))
    {
        LOG(L"Shader {} is out of date!", GetName());
        return Compile();
    }

    bool success = true;
    if (serializedRootSignatureSize > 0ull)
    {
        const HRESULT result = D3DCreateBlob(serializedRootSignatureSize, _serializedRootSignature.GetAddressOf());
        if (FAILED(result))
        {
            LOG(L"Failed to create serialized root signature blob for shader {}!", GetName());
            success = false;
        }
        else
        {
            reader.Read(static_cast<std::byte*>(_serializedRootSignature->GetBufferPointer()), serializedRootSignatureSize);
        }
    }

    if (vertexShaderSize > 0ull)
    {
        const HRESULT result = D3DCreateBlob(vertexShaderSize, _vertexShader.GetAddressOf());
        if (FAILED(result))
        {
            LOG(L"Failed to create vertex shader blob for shader {}!", GetName());
            success = false;
        }
        else
        {
            reader.Read(static_cast<std::byte*>(_vertexShader->GetBufferPointer()), vertexShaderSize);
        }
    }

    if (pixelShaderSize > 0)
    {
        const HRESULT result = D3DCreateBlob(pixelShaderSize, _pixelShader.GetAddressOf());
        if (FAILED(result))
        {
            LOG(L"Failed to create pixel shader blob for shader {}!", GetName());
            success = false;
        }
        else
        {
            reader.Read(static_cast<std::byte*>(_pixelShader->GetBufferPointer()), pixelShaderSize);
        }
    }

    if (success)
    {
        if (!Initialize())
        {
            LOG(L"Failed to deserialize shader {}!", GetName());
            return false;
        }
    }
    else
    {
        return Compile();
    }

    return true;
}

const D3D12_ROOT_SIGNATURE_DESC& DX12Shader::GetRootSignatureDesc(PassKey<DX12RenderingSubsystem>) const
{
    return _rootSignatureDesc;
}

bool DX12Shader::Compile()
{
    LOG(L"Compiling shader {}...", GetName());
    const std::filesystem::path importPath = GetImportPath();

    _lastCompileTime = last_write_time(importPath);

    const ComPtr<ID3DBlob> vertexShader = CompileShader(importPath, nullptr, "VS", "vs_5_1");
    if (vertexShader == nullptr)
    {
        return false;
    }

    const ComPtr<ID3DBlob> pixelShader = CompileShader(importPath, nullptr, "PS", "ps_5_1");
    if (pixelShader == nullptr)
    {
        return false;
    }

    _vertexShader = vertexShader;
    _pixelShader = pixelShader;

    ID3D12ShaderReflection* vertexShaderReflection = nullptr;
    ID3D12ShaderReflection* pixelShaderReflection = nullptr;


    CD3DX12_ROOT_PARAMETER slotRootParameter[1];

    CD3DX12_DESCRIPTOR_RANGE cbvTable;
    cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1,
                                            slotRootParameter,
                                            0,
                                            nullptr,
                                            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> errorBlob = nullptr;
    const HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc,
                                                   D3D_ROOT_SIGNATURE_VERSION_1,
                                                   _serializedRootSignature.GetAddressOf(),
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

    if (!Initialize())
    {
        LOG(L"Failed to initialize shader {}!", GetName());
        return false;
    }

    LOG(L"Shader {} compiled successfully!", GetName());

    MarkDirtyForAutosave();

    return true;
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

bool DX12Shader::InitializeRootSignature(const DX12RenderingSubsystem& renderingSubsystem)
{
    if (_serializedRootSignature == nullptr)
    {
        return false;
    }

    const HRESULT result = renderingSubsystem.GetDevice()->CreateRootSignature(
        0,
        _serializedRootSignature->GetBufferPointer(),
        _serializedRootSignature->GetBufferSize(),
        IID_PPV_ARGS(&_rootSignature));

    if (FAILED(result))
    {
        LOG(L"Failed to create root signature for shader {}!", GetName());

        return false;
    }

    return true;
}

void DX12Shader::InitializePSO(const DX12RenderingSubsystem& renderingSubsystem)
{
    if (_vertexShader == nullptr || _pixelShader == nullptr)
    {
        return;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = {_inputLayout.data(), static_cast<uint32>(_inputLayout.size())};
    psoDesc.pRootSignature = _rootSignature.Get();
    psoDesc.VS =
    {
        static_cast<BYTE*>(_vertexShader->GetBufferPointer()),
        _vertexShader->GetBufferSize()
    };
    psoDesc.PS =
    {
        static_cast<BYTE*>(_pixelShader->GetBufferPointer()),
        _pixelShader->GetBufferSize()
    };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = renderingSubsystem.GetFrameBufferFormat();
    psoDesc.SampleDesc.Count = renderingSubsystem.GetMSAASampleCount();
    psoDesc.SampleDesc.Quality = renderingSubsystem.GetMSAAQuality() - 1;
    psoDesc.DSVFormat = renderingSubsystem.GetDepthStencilFormat();
    ThrowIfFailed(renderingSubsystem.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&_pso)));
}
