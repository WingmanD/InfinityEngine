#include "DX12ShaderBase.h"

#include "DX12RenderingSubsystem.h"
#include "TypeRegistry.h"

uint32 DX12ShaderBase::GetStructuredBufferSlotIndex(Name name) const
{
    auto it = StructuredBufferParameters.FindIf([&name](const StructuredBufferParameter& parameter)
    {
        return parameter.BufferName == name;
    });

    if (it != StructuredBufferParameters.end())
    {
        return it->SlotIndex;
    }

    DEBUG_BREAK();
    return -1;
}

const D3D12_ROOT_SIGNATURE_DESC& DX12ShaderBase::GetRootSignatureDesc(PassKey<DX12RenderingSubsystem>) const
{
    return RootSignatureDesc;
}

ComPtr<IDxcResult> DX12ShaderBase::CompileShader(const std::filesystem::path& shaderPath,
                                                 const std::wstring& entryPoint, const std::wstring& target,
                                                 const std::vector<D3D_SHADER_MACRO>& defines)
{
    std::ifstream fileStream(shaderPath);
    if (!fileStream.is_open())
    {
        LOG(L"ERROR: failed to open shader file: {}", shaderPath.wstring());
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

bool DX12ShaderBase::InitializeRootSignature(const DX12RenderingSubsystem& renderingSubsystem)
{
    if (SerializedRootSignature == nullptr)
    {
        return false;
    }

    const HRESULT result = renderingSubsystem.GetDevice()->CreateRootSignature(
        0,
        SerializedRootSignature->GetBufferPointer(),
        SerializedRootSignature->GetBufferSize(),
        IID_PPV_ARGS(&RootSignature));

    if (FAILED(result))
    {
        LOG(L"ERROR: failed to create root signature!");

        return false;
    }

    return true;
}

bool DX12ShaderBase::ReflectShaderParameters(IDxcResult* compileResult, ShaderReflection& reflection) const
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
        LOG(L"Failed to reflect shader!");
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
                bool alreadyReflected = false;
                for (const MaterialParameterDescriptor& constantBufferParameterType : reflection.ConstantBufferParameterTypes)
                {
                    if (constantBufferParameterType.Name == bindDesc.Name)
                    {
                        alreadyReflected = true;
                        break;
                    }
                }

                if (alreadyReflected)
                {
                    break;
                }
                
                for (const ShaderReflection::RootConstant& rootConstantParameter : reflection.RootConstantParameters)
                {
                    if (rootConstantParameter.ParameterName == Name(Util::ToWString(bindDesc.Name)))
                    {
                        alreadyReflected = true;
                        break;
                    }
                } 

                if (alreadyReflected)
                {
                    break;
                }
                
                if (!ReflectConstantBuffer(shaderReflection.Get(), bindDesc, reflection))
                {
                    return false;
                }

                break;
            }
        case D3D_SIT_STRUCTURED:
            {
                StructuredBufferParameter structuredBufferParameter;
                structuredBufferParameter.BufferName = Name(Util::ToWString(bindDesc.Name));
                structuredBufferParameter.SlotIndex = static_cast<uint32>(reflection.RootParameters.Count());
                const auto result = reflection.StructuredBufferParameterTypes.insert(structuredBufferParameter);

                if (result.second)
                {
                    D3D12_ROOT_PARAMETER parameter = {};
                    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
                    parameter.Descriptor.ShaderRegister = bindDesc.BindPoint;
                    parameter.Descriptor.RegisterSpace = bindDesc.Space;
                    parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                    reflection.RootParameters.Add(parameter);
                }

                break;
            }
        case D3D_SIT_UAV_RWSTRUCTURED:
        case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
            {
                StructuredBufferParameter structuredBufferParameter;
                structuredBufferParameter.BufferName = Name(Util::ToWString(bindDesc.Name));
                structuredBufferParameter.SlotIndex = static_cast<uint32>(reflection.RootParameters.Count());
                const auto result = reflection.StructuredBufferParameterTypes.insert(structuredBufferParameter);

                if (result.second)
                {
                    D3D12_ROOT_PARAMETER parameter = {};
                    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
                    parameter.Descriptor.ShaderRegister = bindDesc.BindPoint;
                    parameter.Descriptor.RegisterSpace = bindDesc.Space;
                    parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                    reflection.RootParameters.Add(parameter);
                }

                break;
            }
        case D3D_SIT_TEXTURE:
            {
                // todo
                LOG(L"Texture '{}' found!", Util::ToWString(bindDesc.Name));
                break;
            }
        case D3D_SIT_SAMPLER:
            {
                // todo
                LOG(L"Sampler '{}' found!", Util::ToWString(bindDesc.Name));
                break;
            }
        default:
            {
                LOG(L"Warning: Unsupported shader resource type '{}'!", Util::ToWString(bindDesc.Name));
                break;
            }
        }
    }

    return true;
}

bool DX12ShaderBase::ReflectConstantBuffer(ID3D12ShaderReflection* shaderReflection,
                                           const D3D12_SHADER_INPUT_BIND_DESC& bindDesc,
                                           ShaderReflection& reflection) const
{
    ID3D12ShaderReflectionConstantBuffer* cbReflection = shaderReflection->GetConstantBufferByIndex(
        static_cast<uint32>(reflection.ConstantBufferParameterTypes.size() + reflection.RootConstantParameters.size())
    );
    if (cbReflection == nullptr)
    {
        LOG(L"ERROR: failed to reflect constant buffer {}!", Util::ToWString(bindDesc.Name));
        return false;
    }

    D3D12_SHADER_BUFFER_DESC bufferDesc;
    HRESULT hr = cbReflection->GetDesc(&bufferDesc);

    if (FAILED(hr))
    {
        LOG(L"ERROR: failed to get constant buffer description for {}!", Util::ToWString(bindDesc.Name));
        return false;
    }

    for (UINT i = 0; i < bufferDesc.Variables; i++)
    {
        ID3D12ShaderReflectionVariable* varReflection = cbReflection->GetVariableByIndex(i);
        if (varReflection == nullptr)
        {
            LOG(L"ERROR: failed to reflect variable {} in constant buffer {}!",
                i,
                Util::ToWString(bindDesc.Name));
            return false;
        }

        ID3D12ShaderReflectionType* typeReflection = varReflection->GetType();
        D3D12_SHADER_TYPE_DESC typeDesc;
        hr = typeReflection->GetDesc(&typeDesc);

        if (FAILED(hr))
        {
            LOG(L"ERROR: failed to get type description for variable {} in constant buffer {}!",
                i,
                Util::ToWString(bindDesc.Name));
            return false;
        }
        
        const Type* type = TypeRegistry::Get().FindTypeByName(typeDesc.Name);
        if (type == nullptr)
        {
            LOG(L"Note: failed to find type {}! Converting it to root constants!", Util::ToWString(typeDesc.Name));
            
            const auto result = reflection.RootConstantParameters.insert({
                Name(Util::ToWString(typeDesc.Name)),
                static_cast<uint32>(reflection.RootParameters.Count())
            });

            if (result.second)
            {
                D3D12_ROOT_PARAMETER parameter = {};
                parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
                parameter.Constants.ShaderRegister = bindDesc.BindPoint;
                parameter.Constants.RegisterSpace = bindDesc.Space;
                parameter.Constants.Num32BitValues = bufferDesc.Size / 4;

                reflection.RootParameters.Add(parameter);
            }

            continue;
        }

        MaterialParameterDescriptor parameterType;
        parameterType.Name = bufferDesc.Name;
        parameterType.ParameterType = TypeRegistry::Get().FindTypeByName(typeDesc.Name);
        parameterType.SlotIndex = static_cast<uint32>(reflection.RootParameters.Count());

        const auto result = reflection.ConstantBufferParameterTypes.insert(parameterType);
        if (result.second)
        {
            D3D12_ROOT_PARAMETER parameter = {};
            parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            parameter.Descriptor.ShaderRegister = bindDesc.BindPoint;
            parameter.Descriptor.RegisterSpace = bindDesc.Space;
            parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            reflection.RootParameters.Add(parameter);
        }
    }

    return true;
}

bool DX12ShaderBase::SerializeBase(MemoryWriter& writer) const
{
    if (SerializedRootSignature != nullptr)
    {
        writer << SerializedRootSignature->GetBufferSize();

        writer.Write(static_cast<const std::byte*>(SerializedRootSignature->GetBufferPointer()),
                     SerializedRootSignature->GetBufferSize());
    }
    else
    {
        writer << 0ull;
    }

    writer << StructuredBufferParameters;

    return true;
}

bool DX12ShaderBase::DeserializeBase(MemoryReader& reader)
{
    bool success = true;

    uint64 serializedRootSignatureSize;
    reader >> serializedRootSignatureSize;

    if (serializedRootSignatureSize > 0)
    {
        const HRESULT result = DX12RenderingSubsystem::Get().GetDXCUtils().CreateBlob(
            reader.GetCurrentPointer(),
            static_cast<uint32>(serializedRootSignatureSize),
            DXC_CP_ACP,
            SerializedRootSignature.GetAddressOf());

        if (FAILED(result))
        {
            LOG(L"ERROR: failed to create serialized root signature blob!");
            success = false;
        }
        reader.Skip(serializedRootSignatureSize);
    }

    reader >> StructuredBufferParameters;

    return success;
}

MemoryReader& operator>>(MemoryReader& reader, DX12ShaderBase::StructuredBufferParameter& parameter)
{
    reader >> parameter.BufferName;
    reader >> parameter.SlotIndex;

    return reader;
}

MemoryWriter& operator<<(MemoryWriter& writer, const DX12ShaderBase::StructuredBufferParameter& parameter)
{
    writer << parameter.BufferName;
    writer << parameter.SlotIndex;

    return writer;
}
