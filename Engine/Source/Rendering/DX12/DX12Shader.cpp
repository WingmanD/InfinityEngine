#include "DX12Shader.h"
#include <d3dcompiler.h>
#include <filesystem>
#include "Core.h"
#include "d3dx12.h"
#include "DX12MaterialParameterMap.h"
#include "DX12RenderingSubsystem.h"
#include "Util.h"
#include "Engine/Engine.h"
#include "Engine/Subsystems/AssetManager.h"

std::vector<D3D12_INPUT_ELEMENT_DESC> DX12Shader::_inputLayout = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
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

void DX12Shader::Apply(ID3D12GraphicsCommandList* commandList) const
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

    if (!shader->Recompile(true))
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

    // todo we need to store information about what was serialized - if something is missing, we will try to read too much data
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

    if (ParameterMap != nullptr)
    {
        ParameterMap->Serialize(writer);
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
        return Recompile(true);
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

    ParameterMap = std::make_unique<DX12MaterialParameterMap>();
    if (!ParameterMap->Deserialize(reader))
    {
        success = false;
        ParameterMap = nullptr;
    }

    if (success)
    {
        if (!Initialize())
        {
            LOG(L"Failed to deserialize shader {}!", GetName());
            return false;
        }

        const DX12RenderingSubsystem& renderingSubsystem = static_cast<DX12RenderingSubsystem&>(RenderingSubsystem::Get());
        if (!InitializeRootSignature(renderingSubsystem))
        {
            return false;
        }

        InitializePSO(renderingSubsystem);
    }
    else
    {
        return Recompile(true);
    }

    return true;
}

const D3D12_ROOT_SIGNATURE_DESC& DX12Shader::GetRootSignatureDesc(PassKey<DX12RenderingSubsystem>) const
{
    return _rootSignatureDesc;
}

bool DX12Shader::Recompile(bool immediate)
{
    if (_beingRecompiled)
    {
        return false;
    }

    _beingRecompiled = true;

    DX12RenderingSubsystem& renderingSubsystem = dynamic_cast<DX12RenderingSubsystem&>(RenderingSubsystem::Get());

    LOG(L"Compiling shader {}...", GetName());
    const std::filesystem::path& importPath = GetImportPath();

    std::shared_ptr<RecompiledData> recompiledData = std::make_shared<RecompiledData>();

    recompiledData->LastCompileTime = last_write_time(importPath);

    const ComPtr<ID3DBlob> vertexShader = CompileShader(importPath, nullptr, "VS", "vs_5_1");
    if (vertexShader == nullptr)
    {
        _beingRecompiled = false;
        return false;
    }

    const ComPtr<ID3DBlob> pixelShader = CompileShader(importPath, nullptr, "PS", "ps_5_1");
    if (pixelShader == nullptr)
    {
        _beingRecompiled = false;
        return false;
    }

    std::vector<D3D12_ROOT_PARAMETER> rootParameters;
    std::set<MaterialParameterDescriptor> constantBufferParameterTypes;

    if (!ReflectShaderParameters(vertexShader.Get(), rootParameters, constantBufferParameterTypes))
    {
        LOG(L"Failed to compile shader {} - failed to reflect vertex shader", GetName());
        _beingRecompiled = false;
        return false;
    }

    if (!ReflectShaderParameters(pixelShader.Get(), rootParameters, constantBufferParameterTypes))
    {
        LOG(L"Failed to compile shader {} - failed to reflect pixel shader", GetName());
        return false;
    }

    recompiledData->VertexShader = vertexShader;
    recompiledData->PixelShader = pixelShader;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.NumParameters = static_cast<uint32>(rootParameters.size());
    rootSignatureDesc.pParameters = rootParameters.data();
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> errorBlob = nullptr;
    const HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
                                                   D3D_ROOT_SIGNATURE_VERSION_1,
                                                   recompiledData->SerializedRootSignature.GetAddressOf(),
                                                   errorBlob.GetAddressOf());
    if (errorBlob != nullptr)
    {
        OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
    }

    if (FAILED(hr))
    {
        LOG(L"Failed to serialize root signature for shader {}!", GetName());
        _beingRecompiled = false;
        return false;
    }

    if (!Initialize())
    {
        LOG(L"Failed to initialize shader {}!", GetName());
        _beingRecompiled = false;
        return false;
    }

    HRESULT result = renderingSubsystem.GetDevice()->CreateRootSignature(
        0,
        recompiledData->SerializedRootSignature->GetBufferPointer(),
        recompiledData->SerializedRootSignature->GetBufferSize(),
        IID_PPV_ARGS(&recompiledData->RootSignature));

    if (FAILED(result))
    {
        LOG(L"Failed to create root signature for shader {}!", GetName());

        _beingRecompiled = false;
        return false;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = {_inputLayout.data(), static_cast<uint32>(_inputLayout.size())};
    psoDesc.pRootSignature = recompiledData->RootSignature.Get();
    psoDesc.VS =
    {
        static_cast<BYTE*>(recompiledData->VertexShader->GetBufferPointer()),
        recompiledData->VertexShader->GetBufferSize()
    };
    psoDesc.PS =
    {
        static_cast<BYTE*>(recompiledData->PixelShader->GetBufferPointer()),
        recompiledData->PixelShader->GetBufferSize()
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

    result = renderingSubsystem.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&recompiledData->Pso));
    if (FAILED(result))
    {
        LOG(L"Failed to create PSO for shader {}!", GetName());
        _beingRecompiled = false;
        return false;
    }

    recompiledData->ParameterMap = std::make_unique<DX12MaterialParameterMap>();
    if (!recompiledData->ParameterMap->Initialize(constantBufferParameterTypes))
    {
        _beingRecompiled = false;
        return false;
    }

    std::weak_ptr weakThis = shared_from_this();
    auto lambda = [weakThis, recompiledData](RenderingSubsystem* renderingSubsystem)
    {
        std::shared_ptr<DX12Shader> shader = std::static_pointer_cast<DX12Shader>(weakThis.lock());
        if (shader == nullptr)
        {
            return;
        }

        shader->_serializedRootSignature = std::move(recompiledData->SerializedRootSignature);
        shader->_rootSignature = std::move(recompiledData->RootSignature);
        shader->_rootSignatureDesc = std::move(recompiledData->RootSignatureDesc);
        shader->_pso = std::move(recompiledData->Pso);
        shader->_vertexShader = std::move(recompiledData->VertexShader);
        shader->_pixelShader = std::move(recompiledData->PixelShader);
        shader->_lastCompileTime = std::move(recompiledData->LastCompileTime);
        shader->ParameterMap = std::move(recompiledData->ParameterMap);
        shader->_beingRecompiled = false;

        shader->MarkDirtyForAutosave();

        shader->OnRecompiled.Broadcast(shader.get());
    };

    if (immediate)
    {
        lambda(&renderingSubsystem);
    }
    else
    {
        renderingSubsystem.GetEventQueue().Enqueue(lambda);
    }

    LOG(L"Shader {} compiled successfully!", GetName());

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

bool DX12Shader::InitializePSO(const DX12RenderingSubsystem& renderingSubsystem)
{
    if (_vertexShader == nullptr || _pixelShader == nullptr)
    {
        return false;
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
    const HRESULT result = renderingSubsystem.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&_pso));
    if (FAILED(result))
    {
        LOG(L"Failed to create PSO for shader {}!", GetName());
        return false;
    }

    return true;
}

bool DX12Shader::ReflectShaderParameters(ID3DBlob* shaderBlob, std::vector<D3D12_ROOT_PARAMETER>& rootParameters, std::set<MaterialParameterDescriptor>& constantBufferParameterTypes)
{
    ID3D12ShaderReflection* shaderReflection = nullptr;
    D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), IID_ID3D12ShaderReflection, reinterpret_cast<void**>(&shaderReflection));

    if (shaderReflection == nullptr)
    {
        LOG(L"Failed to reflect shader {}!", GetName());
        return false;
    }

    D3D12_SHADER_DESC shaderDesc;
    shaderReflection->GetDesc(&shaderDesc);

    for (UINT i = 0; i < shaderDesc.BoundResources; i++)
    {
        D3D12_SHADER_INPUT_BIND_DESC bindDesc;
        shaderReflection->GetResourceBindingDesc(i, &bindDesc);

        switch (bindDesc.Type)
        {
            case D3D_SIT_CBUFFER:
            {
                if (!ReflectConstantBuffer(shaderReflection, bindDesc, rootParameters, constantBufferParameterTypes))
                {
                    return false;
                }

                break;
            }
            case D3D_SIT_TEXTURE:
            {
                // todo
                LOG(L"Texture '{}' found in shader {}!", Util::ToWString(bindDesc.Name), GetName());
                break;
            }
            case D3D_SIT_SAMPLER:
            {
                // todo
                LOG(L"Sampler '{}' found in shader {}!", Util::ToWString(bindDesc.Name), GetName());
                break;
            }
            default:
            {
                LOG(L"Unsupported shader resource type '{}' in shader {}!", Util::ToWString(bindDesc.Name), GetName());
                break;
            }
        }
    }

    return true;
}

bool DX12Shader::ReflectConstantBuffer(ID3D12ShaderReflection* shaderReflection, const D3D12_SHADER_INPUT_BIND_DESC& bindDesc, std::vector<D3D12_ROOT_PARAMETER>& rootParameters, std::set<MaterialParameterDescriptor>& constantBufferParameterTypes) const
{
    ID3D12ShaderReflectionConstantBuffer* cbReflection = shaderReflection->GetConstantBufferByIndex(bindDesc.BindPoint);
    if (cbReflection == nullptr)
    {
        LOG(L"Failed to reflect constant buffer {} in shader {}!", Util::ToWString(bindDesc.Name), GetName());
        return false;
    }

    D3D12_SHADER_BUFFER_DESC bufferDesc;
    cbReflection->GetDesc(&bufferDesc);

    for (UINT i = 0; i < bufferDesc.Variables; i++)
    {
        ID3D12ShaderReflectionVariable* varReflection = cbReflection->GetVariableByIndex(i);
        if (varReflection == nullptr)
        {
            LOG(L"Failed to reflect variable {} in constant buffer {} in shader {}!", i, Util::ToWString(bindDesc.Name), GetName());
            return false;
        }

        ID3D12ShaderReflectionType* typeReflection = varReflection->GetType();
        D3D12_SHADER_TYPE_DESC typeDesc;
        typeReflection->GetDesc(&typeDesc);

        const Type* type = TypeRegistry::Get().FindTypeByName(typeDesc.Name);
        if (type == nullptr)
        {
            LOG(L"Failed to find type {} in shader {}!", Util::ToWString(typeDesc.Name), GetName());
            return false;
        }

        MaterialParameterDescriptor parameterType;
        parameterType.Name = bufferDesc.Name;
        parameterType.ParameterType = TypeRegistry::Get().FindTypeByName(typeDesc.Name);
        parameterType.SlotIndex = bindDesc.BindPoint;

        const auto result = constantBufferParameterTypes.insert(parameterType);
        if (result.second)
        {
            D3D12_ROOT_PARAMETER parameter = {};
            parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            parameter.Descriptor.ShaderRegister = bindDesc.BindPoint;
            parameter.Descriptor.RegisterSpace = bindDesc.Space;
            parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParameters.push_back(parameter);
        }
    }

    return true;
}
