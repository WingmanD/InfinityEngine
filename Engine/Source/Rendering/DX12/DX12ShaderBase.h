#pragma once

#include "Name.h"
#include "Containers/DArray.h"
#include "Rendering/DX12/DX12RenderingCore.h"
#include "Rendering/MaterialParameterMap.h"
#include "MemoryReader.h"
#include "MemoryWriter.h"
#include "Object.h"
#include <d3d12shader.h>
#include <set>

class DX12RenderingSubsystem;

class DX12ShaderBase
{
public:
    struct StructuredBufferParameter
    {
        Name BufferName;
        uint32 SlotIndex;

        auto operator<=>(const StructuredBufferParameter& other) const
        {
            return BufferName <=> other.BufferName;
        }
    };

public:
    virtual ~DX12ShaderBase() = default;

    uint32 GetStructuredBufferSlotIndex(Name name) const;
    const D3D12_ROOT_SIGNATURE_DESC& GetRootSignatureDesc(PassKey<DX12RenderingSubsystem>) const;

protected:

    static ComPtr<IDxcResult> CompileShader(const std::filesystem::path& shaderPath,
                                            const std::wstring& entryPoint,
                                            const std::wstring& target,
                                            const std::vector<D3D_SHADER_MACRO>& defines = {});

    bool InitializeRootSignature(const DX12RenderingSubsystem& renderingSubsystem);
    
    bool ReflectShaderParameters(IDxcResult* compileResult, std::vector<D3D12_ROOT_PARAMETER>& rootParameters, std::set<MaterialParameterDescriptor>& constantBufferParameterTypes, std::set<StructuredBufferParameter>& structuredBufferParameterTypes) const;
    bool ReflectConstantBuffer(ID3D12ShaderReflection* shaderReflection, const D3D12_SHADER_INPUT_BIND_DESC& bindDesc, std::vector<D3D12_ROOT_PARAMETER>& rootParameters, std::set<MaterialParameterDescriptor>& constantBufferParameterTypes) const;

    bool SerializeBase(MemoryWriter& writer) const;
    bool DeserializeBase(MemoryReader& reader);
    
protected:
    ComPtr<ID3D12PipelineState> PSO;

    ComPtr<ID3D12RootSignature> RootSignature;
    D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc{};
    ComPtr<IDxcBlobEncoding> SerializedRootSignature;


    DArray<StructuredBufferParameter, 4> StructuredBufferParameters;
};

MemoryReader& operator>>(MemoryReader& reader, DX12ShaderBase::StructuredBufferParameter& parameter);
MemoryWriter& operator<<(MemoryWriter& writer, const DX12ShaderBase::StructuredBufferParameter& parameter);
