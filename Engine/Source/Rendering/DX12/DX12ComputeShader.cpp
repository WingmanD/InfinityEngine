#include "DX12ComputeShader.h"

#include "DX12RenderingSubsystem.h"

bool DX12ComputeShader::Recompile(bool immediate)
{
    /*
    DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();
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

    std::vector<D3D12_ROOT_PARAMETER> rootParameters;
    std::set<MaterialParameterDescriptor> constantBufferParameterTypes;
    std::set<StructuredBufferParameter> structuredBufferParameterTypes;

    if (!ReflectShaderParameters(vertexShaderResult.Get(), rootParameters, constantBufferParameterTypes,
                                 structuredBufferParameterTypes))
    {
        LOG(L"Failed to compile shader {} - failed to reflect vertex shader", GetName().ToString());
        _beingRecompiled = false;
        return false;
    }

    if (!ReflectShaderParameters(pixelShaderResult.Get(), rootParameters, constantBufferParameterTypes,
                                 structuredBufferParameterTypes))
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
    rootSignatureDesc.pParameters = rootParameters.data(); // todo SRV needs to be bound here
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

    recompiledData->StructuredBufferParameterTypes.Append(structuredBufferParameterTypes);

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
        shader->_structuredBufferParameters = std::move(recompiledData->StructuredBufferParameterTypes);
        shader->_beingRecompiled = false;

        for (const StructuredBufferParameter& structuredBufferParameterType : recompiledData->StructuredBufferParameterTypes)
        {
            if (structuredBufferParameterType.BufferName == Name(L"GInstanceBuffer"))
            {
                shader->_instanceBufferSlotIndex = structuredBufferParameterType.SlotIndex;
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
*/
    return true;
}
