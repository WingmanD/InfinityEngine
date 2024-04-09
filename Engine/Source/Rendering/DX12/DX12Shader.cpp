#include "DX12Shader.h"
#include <filesystem>
#include "Core.h"
#include "d3dx12/d3dx12.h"
#include "DX12MaterialParameterMap.h"
#include "DX12RenderingSubsystem.h"
#include "Util.h"
#include "Engine/Subsystems/AssetManager.h"

std::vector<D3D12_INPUT_ELEMENT_DESC> DX12Shader::_inputLayout = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
};

DX12Shader::DX12Shader()
{
    SetImporterType(DX12ShaderImporter::StaticType());
}

DX12Shader::DX12Shader(Name name) : Shader(name)
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

void DX12Shader::Apply(DX12GraphicsCommandList* commandList) const
{
    commandList->SetPipelineState(_pso.Get());
    commandList->SetGraphicsRootSignature(_rootSignature.Get());
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
    IDxcUtils& dxcUtils = renderingSubsystem.GetDXCUtils();

    LOG(L"Compiling shader {}...", GetName().ToString());
    const std::filesystem::path& importPath = GetImportPath();

    std::shared_ptr<RecompiledData> recompiledData = std::make_shared<RecompiledData>();

    recompiledData->LastCompileTime = last_write_time(importPath);

    const ComPtr<IDxcResult> vertexShaderResult = CompileShader(importPath, L"VS", L"vs_6_8");
    if (vertexShaderResult == nullptr)
    {
        _beingRecompiled = false;
        return false;
    }

    ComPtr<ID3DBlob> vertexShader = nullptr;
    vertexShaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(vertexShader.GetAddressOf()), nullptr);
    if (vertexShader == nullptr)
    {
        LOG(L"Failed to compile shader {} - failed to get vertex shader byte code", GetName().ToString());
        _beingRecompiled = false;
        return false;
    }

    const ComPtr<IDxcResult> pixelShaderResult = CompileShader(importPath, L"PS", L"ps_6_8");
    if (pixelShaderResult == nullptr)
    {
        _beingRecompiled = false;
        return false;
    }

    ComPtr<ID3DBlob> pixelShader = nullptr;
    pixelShaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(pixelShader.GetAddressOf()), nullptr);
    if (pixelShader == nullptr)
    {
        LOG(L"Failed to compile shader {} - failed to get pixelShader shader byte code", GetName().ToString());
        _beingRecompiled = false;
        return false;
    }

    std::vector<D3D12_ROOT_PARAMETER> rootParameters;
    std::set<MaterialParameterDescriptor> constantBufferParameterTypes;

    if (!ReflectShaderParameters(vertexShaderResult.Get(), rootParameters, constantBufferParameterTypes))
    {
        LOG(L"Failed to compile shader {} - failed to reflect vertex shader", GetName().ToString());
        _beingRecompiled = false;
        return false;
    }

    if (!ReflectShaderParameters(pixelShaderResult.Get(), rootParameters, constantBufferParameterTypes))
    {
        LOG(L"Failed to compile shader {} - failed to reflect pixel shader", GetName().ToString());
        return false;
    }

    dxcUtils.CreateBlob(
        vertexShader->GetBufferPointer(),
        static_cast<uint32>(vertexShader->GetBufferSize()),
        DXC_CP_ACP,
        &recompiledData->VertexShader
    );

    dxcUtils.CreateBlob(
        pixelShader->GetBufferPointer(),
        static_cast<uint32>(pixelShader->GetBufferSize()),
        DXC_CP_ACP,
        &recompiledData->PixelShader
    );

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.NumParameters = static_cast<uint32>(rootParameters.size());
    rootSignatureDesc.pParameters = rootParameters.data();  // todo SRV needs to be bound here
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> serializedRootSignature = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    const HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
                                                   D3D_ROOT_SIGNATURE_VERSION_1,
                                                   serializedRootSignature.GetAddressOf(),
                                                   errorBlob.GetAddressOf());
    if (errorBlob != nullptr)
    {
        OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
    }

    if (FAILED(hr))
    {
        LOG(L"Failed to serialize root signature for shader {}!", GetName().ToString());
        _beingRecompiled = false;
        return false;
    }

    if (!Initialize())
    {
        LOG(L"Failed to initialize shader {}!", GetName().ToString());
        _beingRecompiled = false;
        return false;
    }

    HRESULT result = renderingSubsystem.GetDevice()->CreateRootSignature(
        0,
        serializedRootSignature->GetBufferPointer(),
        serializedRootSignature->GetBufferSize(),
        IID_PPV_ARGS(&recompiledData->RootSignature));

    if (FAILED(result))
    {
        LOG(L"Failed to create root signature for shader {}!", GetName().ToString());

        _beingRecompiled = false;
        return false;
    }

    dxcUtils.CreateBlob(
        serializedRootSignature->GetBufferPointer(),
        static_cast<uint32>(serializedRootSignature->GetBufferSize()),
        DXC_CP_ACP,
        &recompiledData->SerializedRootSignature
    );

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
        LOG(L"Failed to create PSO for shader {}!", GetName().ToString());
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
        const std::shared_ptr<DX12Shader> shader = std::static_pointer_cast<DX12Shader>(weakThis.lock());
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

    LOG(L"Shader {} compiled successfully!", GetName().ToString());

    return true;
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
        writer.Write(static_cast<const std::byte*>(_serializedRootSignature->GetBufferPointer()),
                     _serializedRootSignature->GetBufferSize());
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
        LOG(L"Shader {} is out of date!", GetName().ToString());
        return Recompile(true);
    }

    const DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();

    IDxcUtils& dxcUtils = renderingSubsystem.GetDXCUtils();

    bool success = true;
    if (serializedRootSignatureSize > 0ull)
    {
        const HRESULT result = dxcUtils.CreateBlob(
            reader.GetCurrentPointer(),
            static_cast<uint32>(serializedRootSignatureSize),
            DXC_CP_ACP,
            _serializedRootSignature.GetAddressOf());

        if (FAILED(result))
        {
            LOG(L"Failed to create serialized root signature blob for shader {}!", GetName().ToString());
            success = false;
        }

        reader.Skip(serializedRootSignatureSize);
    }

    if (vertexShaderSize > 0ull)
    {
        const HRESULT result = dxcUtils.CreateBlob(
            reader.GetCurrentPointer(),
            static_cast<uint32>(vertexShaderSize),
            DXC_CP_ACP,
            _vertexShader.GetAddressOf());
        if (FAILED(result))
        {
            LOG(L"Failed to create vertex shader blob for shader {}!", GetName().ToString());
            success = false;
        }

        reader.Skip(vertexShaderSize);
    }

    if (pixelShaderSize > 0)
    {
        const HRESULT result = dxcUtils.CreateBlob(
            reader.GetCurrentPointer(),
            static_cast<uint32>(pixelShaderSize),
            DXC_CP_ACP,
            _pixelShader.GetAddressOf());
        if (FAILED(result))
        {
            LOG(L"Failed to create pixel shader blob for shader {}!", GetName().ToString());
            success = false;
        }

        reader.Skip(pixelShaderSize);
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
            LOG(L"Failed to deserialize shader {}!", GetName().ToString());
            return false;
        }

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

std::vector<std::shared_ptr<Asset>> DX12Shader::Import(const std::shared_ptr<Importer>& importer) const
{
    const std::shared_ptr<DX12ShaderImporter> shaderImporter = std::dynamic_pointer_cast<DX12ShaderImporter>(importer);
    if (shaderImporter == nullptr)
    {
        return {};
    }

    const std::filesystem::path& path = shaderImporter->Path;

    if (path.empty() || !exists(path))
    {
        LOG(L"Invalid shader path: {}", path.wstring());
        return {};
    }

    if (path.extension() != ".hlsl")
    {
        LOG(L"Invalid shader extension: {}.", path.wstring());
        return {};
    }

    AssetManager& assetManager = AssetManager::Get();

    std::shared_ptr<DX12Shader> shader = assetManager.NewAsset<DX12Shader>(Name(path.stem().wstring()));
    shader->SetImportPath(path);

    if (!shader->Recompile(true))
    {
        LOG(L"Imported shader {} failed to compile!", shader->GetName().ToString());
    }

    shader->MarkDirtyForAutosave();

    return {shader};
}

ComPtr<IDxcResult> DX12Shader::CompileShader(const std::filesystem::path& shaderPath,
                                             const std::wstring& entryPoint,
                                             const std::wstring& target,
                                             const std::vector<D3D_SHADER_MACRO>& defines /*= {}*/)
{
    std::ifstream fileStream(shaderPath);
    if (!fileStream.is_open())
    {
        LOG(L"Failed to open shader file: {}", shaderPath.wstring());
        return nullptr;
    }

    const std::string contents((std::istreambuf_iterator(fileStream)), std::istreambuf_iterator<char>());

    const DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();

    DXCompiler& d3dCompiler = renderingSubsystem.GetD3DCompiler();
    IDxcUtils& dxcUtils = renderingSubsystem.GetDXCUtils();

    ComPtr<IDxcBlobEncoding> source;
    dxcUtils.CreateBlob(contents.c_str(), static_cast<uint32>(contents.size()), DXC_CP_UTF8, source.GetAddressOf());

    // todo this is constant, optimize
    std::vector<LPCWSTR> arguments;
    arguments.push_back(L"-E");
    arguments.push_back(entryPoint.c_str());

    arguments.push_back(L"-T");
    arguments.push_back(target.c_str());

    arguments.push_back(L"-I");

    const std::wstring includePath = shaderPath.parent_path().wstring();
    arguments.push_back(includePath.c_str());

    arguments.push_back(L"-Qstrip_debug");
    arguments.push_back(L"-Qstrip_reflect");

    arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
    arguments.push_back(DXC_ARG_DEBUG);

    for (const D3D_SHADER_MACRO& define : defines)
    {
        arguments.push_back(L"-D");
        arguments.push_back(Util::ToWString(define.Name).c_str());
    }

    DxcBuffer sourceBuffer;
    sourceBuffer.Ptr = source->GetBufferPointer();
    sourceBuffer.Size = source->GetBufferSize();
    sourceBuffer.Encoding = 0;

    IDxcIncludeHandler* includeHandler = nullptr;
    dxcUtils.CreateDefaultIncludeHandler(&includeHandler);

    ComPtr<IDxcResult> compileResult;

    const HRESULT result = d3dCompiler.Compile(
        &sourceBuffer,
        arguments.data(),
        static_cast<uint32>(arguments.size()),
        includeHandler,
        IID_PPV_ARGS(compileResult.GetAddressOf())
    );

    if (FAILED(result))
    {
        LOG(L"Shader compilation failed: {}", shaderPath.wstring());
        return nullptr;
    }

    ComPtr<IDxcBlobUtf8> errors;
    compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(errors.GetAddressOf()), nullptr);
    if (errors != nullptr && errors->GetStringLength() > 0)
    {
        LOG(L"ERROR: {}", Util::ToWString(static_cast<char*>(errors->GetBufferPointer())));
        return nullptr;
    }

    return compileResult;
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
        LOG(L"Failed to create root signature for shader {}!", GetName().ToString());

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
        LOG(L"Failed to create PSO for shader {}!", GetName().ToString());
        return false;
    }

    return true;
}

bool DX12Shader::ReflectShaderParameters(IDxcResult* compileResult, std::vector<D3D12_ROOT_PARAMETER>& rootParameters,
                                         std::set<MaterialParameterDescriptor>& constantBufferParameterTypes) const
{
    ComPtr<IDxcBlob> reflectionData;
    compileResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(reflectionData.GetAddressOf()), nullptr);

    DxcBuffer reflectionBuffer;
    reflectionBuffer.Ptr = reflectionData->GetBufferPointer();
    reflectionBuffer.Size = reflectionData->GetBufferSize();
    reflectionBuffer.Encoding = 0;

    ComPtr<ID3D12ShaderReflection> shaderReflection;
    DX12RenderingSubsystem::Get().GetDXCUtils().CreateReflection(&reflectionBuffer,
                                                                 IID_PPV_ARGS(shaderReflection.GetAddressOf()));


    if (shaderReflection == nullptr)
    {
        LOG(L"Failed to reflect shader {}!", GetName().ToString());
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
                if (!ReflectConstantBuffer(shaderReflection.Get(), bindDesc, rootParameters, constantBufferParameterTypes))
                {
                    return false;
                }

                break;
            }
        case D3D_SIT_STRUCTURED:
            {
                D3D12_ROOT_PARAMETER parameter = {};
                parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
                parameter.Descriptor.ShaderRegister = bindDesc.BindPoint;
                parameter.Descriptor.RegisterSpace = bindDesc.Space;
                parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                rootParameters.push_back(parameter);

                // todo fill out SRV parameter descriptions, just as we did with constant buffers
                // we need to know which index to use for instance buffer, but also for material parameter buffer for later
                
                break;
            }
        case D3D_SIT_TEXTURE:
            {
                // todo
                LOG(L"Texture '{}' found in shader {}!", Util::ToWString(bindDesc.Name), GetName().ToString());
                break;
            }
        case D3D_SIT_SAMPLER:
            {
                // todo
                LOG(L"Sampler '{}' found in shader {}!", Util::ToWString(bindDesc.Name), GetName().ToString());
                break;
            }
        default:
            {
                LOG(L"Unsupported shader resource type '{}' in shader {}!", Util::ToWString(bindDesc.Name), GetName().ToString());
                break;
            }
        }
    }

    return true;
}

bool DX12Shader::ReflectConstantBuffer(ID3D12ShaderReflection* shaderReflection,
                                       const D3D12_SHADER_INPUT_BIND_DESC& bindDesc,
                                       std::vector<D3D12_ROOT_PARAMETER>& rootParameters,
                                       std::set<MaterialParameterDescriptor>& constantBufferParameterTypes) const
{
    ID3D12ShaderReflectionConstantBuffer* cbReflection = shaderReflection->GetConstantBufferByIndex(bindDesc.BindPoint);
    if (cbReflection == nullptr)
    {
        LOG(L"Failed to reflect constant buffer {} in shader {}!", Util::ToWString(bindDesc.Name),
            GetName().ToString());
        return false;
    }

    D3D12_SHADER_BUFFER_DESC bufferDesc;
    cbReflection->GetDesc(&bufferDesc);

    for (UINT i = 0; i < bufferDesc.Variables; i++)
    {
        ID3D12ShaderReflectionVariable* varReflection = cbReflection->GetVariableByIndex(i);
        if (varReflection == nullptr)
        {
            LOG(L"Failed to reflect variable {} in constant buffer {} in shader {}!", i, Util::ToWString(bindDesc.Name),
                GetName().ToString());
            return false;
        }

        ID3D12ShaderReflectionType* typeReflection = varReflection->GetType();
        D3D12_SHADER_TYPE_DESC typeDesc;
        typeReflection->GetDesc(&typeDesc);

        const Type* type = TypeRegistry::Get().FindTypeByName(typeDesc.Name);
        if (type == nullptr)
        {
            LOG(L"Failed to find type {} in shader {}!", Util::ToWString(typeDesc.Name), GetName().ToString());
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
