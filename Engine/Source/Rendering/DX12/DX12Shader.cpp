#include "DX12Shader.h"
#include "Core.h"
#include "DX12RenderingCore.h"
#include "d3dx12/d3dx12.h"
#include "DX12MaterialParameterMap.h"
#include "DX12RenderingSubsystem.h"
#include "DynamicGPUBufferUploader.h"
#include "Engine/Subsystems/AssetManager.h"
#include "MaterialParameterTypes.h"
#include "Util.h"
#include <filesystem>
#include <atlstr.h>


std::vector<D3D12_INPUT_ELEMENT_DESC> DX12Shader::_inputLayout = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 60, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
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

void DX12Shader::Apply(DX12GraphicsCommandList* commandList, PassKey<DX12MaterialRenderingData>) const
{
    commandList->SetPipelineState(PSO.Get());
    commandList->SetGraphicsRootSignature(RootSignature.Get());
}

void DX12Shader::BindInstanceBuffers(DX12GraphicsCommandList& commandList,
                                     const AppendStructuredBuffer<SMInstance>& instanceBuffer,
                                     uint32 instanceStart,
                                     const DynamicGPUBuffer<MaterialParameter>& materialBuffer) const
{
    commandList.SetGraphicsRoot32BitConstant(_instanceOffsetSlotIndex, instanceStart, 0);
    
    commandList.SetGraphicsRootShaderResourceView(
        _instanceBufferSlotIndex,
        instanceBuffer.GetSRVGPUVirtualAddress()
    );

    commandList.SetGraphicsRootShaderResourceView(
        _materialBufferSlotIndex,
        materialBuffer.GetProxy<DynamicGPUBufferUploader<MaterialParameter>>()->GetStructuredBuffer().
                       GetSRVGPUVirtualAddress()
    );
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

    const ComPtr<IDxcResult> vertexShaderResult = CompileShader(importPath, L"VS", L"vs_6_6");
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

    const ComPtr<IDxcResult> pixelShaderResult = CompileShader(importPath, L"PS", L"ps_6_6");
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

    if (!ReflectShaderParameters(vertexShaderResult.Get(), recompiledData->Reflection))
    {
        LOG(L"Failed to compile shader {} - failed to reflect vertex shader", GetName().ToString());
        _beingRecompiled = false;
        return false;
    }

    if (!ReflectShaderParameters(pixelShaderResult.Get(), recompiledData->Reflection))
    {
        LOG(L"Failed to compile shader {} - failed to reflect pixel shader", GetName().ToString());
        _beingRecompiled = false;
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
    rootSignatureDesc.NumParameters = static_cast<uint32>(recompiledData->Reflection.RootParameters.Count());
    rootSignatureDesc.pParameters = recompiledData->Reflection.RootParameters.GetData();
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

    if (!InitializePSO(renderingSubsystem, recompiledData->Pso,
                       recompiledData->RootSignature,
                       recompiledData->VertexShader,
                       recompiledData->PixelShader)
    )
    {
        _beingRecompiled = false;
        return false;
    }

    recompiledData->ParameterMap = std::make_unique<DX12MaterialParameterMap>();
    if (!recompiledData->ParameterMap->Initialize(recompiledData->Reflection.ConstantBufferParameterTypes))
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

        shader->SerializedRootSignature = std::move(recompiledData->SerializedRootSignature);
        shader->RootSignature = std::move(recompiledData->RootSignature);
        shader->RootSignatureDesc = std::move(recompiledData->RootSignatureDesc);
        shader->PSO = std::move(recompiledData->Pso);
        shader->_vertexShader = std::move(recompiledData->VertexShader);
        shader->_pixelShader = std::move(recompiledData->PixelShader);
        shader->ParameterMap = std::move(recompiledData->ParameterMap);
        shader->StructuredBufferParameters.Append(recompiledData->Reflection.StructuredBufferParameterTypes);
        shader->_beingRecompiled = false;

        // todo names here will be replaced with types, once struct reflection is implemented
        for (const StructuredBufferParameter& structuredBufferParameterType : recompiledData->Reflection.StructuredBufferParameterTypes)
        {
            if (structuredBufferParameterType.BufferName == Name(L"GInstanceBuffer"))
            {
                shader->_instanceBufferSlotIndex = structuredBufferParameterType.SlotIndex;
            }
            else if (structuredBufferParameterType.BufferName == Name(L"GPointLights"))
            {
                shader->_pointLightsBufferSlotIndex = structuredBufferParameterType.SlotIndex;
            }
            else if (structuredBufferParameterType.BufferName == Name(L"GLightTiles"))
            {
                shader->_tileBufferSlotIndex = structuredBufferParameterType.SlotIndex;
            }
            else if (structuredBufferParameterType.BufferName == Name(L"GLightIndices"))
            {
                shader->_lightIndexBufferSlotIndex = structuredBufferParameterType.SlotIndex;
            }
            else
            {
                std::wstring name = structuredBufferParameterType.BufferName.ToString();
                if (name.starts_with(L"GMP_") || name.starts_with(L"MP_"))
                {
                    shader->_materialBufferSlotIndex = structuredBufferParameterType.SlotIndex;

                    shader->MaterialInstanceDataType = TypeRegistry::Get().FindTypeByName(
                        Util::ToString(name.substr(name.find_first_of(L"_") + 1)));
                }
            }
        }

        for (const ShaderReflection::RootConstant& rootConstantParameter : recompiledData->Reflection.RootConstantParameters)
        {
            if (rootConstantParameter.ParameterName == Name(L"GInstanceOffset"))
            {
                shader->_instanceOffsetSlotIndex = rootConstantParameter.SlotIndex;
                break;
            }
        }
        
        shader->SetLastCompileTime(recompiledData->LastCompileTime);

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

    if (!SerializeBase(writer))
    {
        return false;
    }

    if (_vertexShader != nullptr)
    {
        writer << _vertexShader->GetBufferSize();
        writer.Write(static_cast<const std::byte*>(_vertexShader->GetBufferPointer()), _vertexShader->GetBufferSize());
    }
    else
    {
        writer << 0ull;
    }

    if (_pixelShader != nullptr)
    {
        writer << _pixelShader->GetBufferSize();
        writer.Write(static_cast<const std::byte*>(_pixelShader->GetBufferPointer()), _pixelShader->GetBufferSize());
    }
    else
    {
        writer << 0ull;
    }

    if (ParameterMap != nullptr)
    {
        writer << true;
        ParameterMap->Serialize(writer);
    }
    else
    {
        writer << false;
    }

    return true;
}

bool DX12Shader::Deserialize(MemoryReader& reader)
{
    if (!Shader::Deserialize(reader))
    {
        return false;
    }

    if (!DeserializeBase(reader))
    {
        return false;
    }
    
    if (GetLastCompileTime() < last_write_time(GetImportPath()))
    {
        LOG(L"Shader {} is out of date!", GetName().ToString());
        return Recompile(true);
    }

    bool success = true;
    
    const DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();
    IDxcUtils& dxcUtils = renderingSubsystem.GetDXCUtils();

    uint64 vertexShaderSize;
    reader >> vertexShaderSize;
    
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
    
    uint64 pixelShaderSize;
    reader >> pixelShaderSize;
    
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

    bool hasParameterMap;
    reader >> hasParameterMap;
    
    if (hasParameterMap)
    {
        if (!ParameterMap->Deserialize(reader))
        {
            success = false;
            ParameterMap = nullptr;
        }
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

        for (const StructuredBufferParameter& structuredBufferParameterType : StructuredBufferParameters)
        {
            if (structuredBufferParameterType.BufferName == Name(L"GInstanceBuffer"))
            {
                _instanceBufferSlotIndex = structuredBufferParameterType.SlotIndex;
            }
            else if (structuredBufferParameterType.BufferName == Name(L"GPointLights"))
            {
                _pointLightsBufferSlotIndex = structuredBufferParameterType.SlotIndex;
            }
            else if (structuredBufferParameterType.BufferName == Name(L"GLightTiles"))
            {
                _tileBufferSlotIndex = structuredBufferParameterType.SlotIndex;
            }
            else if (structuredBufferParameterType.BufferName == Name(L"GLightIndices"))
            {
                _lightIndexBufferSlotIndex = structuredBufferParameterType.SlotIndex;
            }
            else
            {
                std::wstring name = structuredBufferParameterType.BufferName.ToString();
                if (name.starts_with(L"GMP_") || name.starts_with(L"MP_"))
                {
                    _materialBufferSlotIndex = structuredBufferParameterType.SlotIndex;

                    MaterialInstanceDataType = TypeRegistry::Get().FindTypeByName(
                        Util::ToString(name.substr(name.find_first_of(L"_") + 1)));
                }
            }
        }

        InitializePSO(renderingSubsystem, PSO, RootSignature, _vertexShader, _pixelShader);
    }
    else
    {
        return Recompile(true);
    }

    return true;
}

DArray<std::shared_ptr<Asset>> DX12Shader::Import(const std::shared_ptr<Importer>& importer) const
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

#if GPU_DEBUG
    arguments.push_back(L"-Qembed_debug");
    arguments.push_back(DXC_ARG_DEBUG);
    arguments.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
#else
    arguments.push_back(L"-Qstrip_debug");
    arguments.push_back(L"-Qstrip_reflect");
#endif

    arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);

    for (const D3D_SHADER_MACRO& define : defines)
    {
        arguments.push_back(L"-D");
        arguments.push_back(Util::ToWString(define.Name).c_str());
    }

    DxcBuffer sourceBuffer;
    sourceBuffer.Ptr = source->GetBufferPointer();
    sourceBuffer.Size = source->GetBufferSize();
    sourceBuffer.Encoding = 0;

    ComPtr<IDxcIncludeHandler> includeHandler = nullptr;
    dxcUtils.CreateDefaultIncludeHandler(&includeHandler);

    ComPtr<IDxcResult> compileResult;

    const HRESULT result = d3dCompiler.Compile(
        &sourceBuffer,
        arguments.data(),
        static_cast<uint32>(arguments.size()),
        includeHandler.Get(),
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

bool DX12Shader::InitializePSO(const DX12RenderingSubsystem& renderingSubsystem,
                               ComPtr<ID3D12PipelineState>& pso,
                               const ComPtr<ID3D12RootSignature>& rootSignature,
                               const ComPtr<IDxcBlobEncoding>& vertexShader,
                               const ComPtr<IDxcBlobEncoding>& pixelShader)
{
    if (vertexShader == nullptr || pixelShader == nullptr)
    {
        return false;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = {_inputLayout.data(), static_cast<uint32>(_inputLayout.size())};
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS =
    {
        static_cast<BYTE*>(vertexShader->GetBufferPointer()),
        vertexShader->GetBufferSize()
    };
    psoDesc.PS =
    {
        static_cast<BYTE*>(pixelShader->GetBufferPointer()),
        pixelShader->GetBufferSize()
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
    const HRESULT result = renderingSubsystem.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
    if (FAILED(result))
    {
        LOG(L"Failed to create PSO for shader {}!", GetName().ToString());
        return false;
    }

    return true;
}
