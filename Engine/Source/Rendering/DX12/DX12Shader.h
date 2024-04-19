#pragma once

#include "PassKey.h"
#include "Rendering/Shader.h"
#include "Importer.h"
#include "Rendering/DX12/DX12RenderingCore.h"
#include "Containers/DynamicGPUBuffer.h"
#include <d3d12shader.h>
#include <set>
#include <wrl/client.h>
#include "DX12Shader.reflection.h"

class DX12MaterialRenderingData;
struct MaterialParameterDescriptor;
struct SMInstance;
using Microsoft::WRL::ComPtr;

class RenderingSubsystem;
class DX12RenderingSubsystem;

REFLECTED()
class DX12ShaderImporter : public Importer
{
    GENERATED()
    
public:
    PROPERTY(Edit)
    std::filesystem::path Path;
};

REFLECTED()
class DX12Shader : public Shader
{
    GENERATED()

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
    DX12Shader();
    DX12Shader(Name name);

    DX12Shader(const DX12Shader& other);
    DX12Shader& operator=(const DX12Shader& other);

    void Apply(DX12GraphicsCommandList* commandList, PassKey<DX12MaterialRenderingData>) const;
    void BindInstanceBuffers(DX12GraphicsCommandList& commandList,
        const DynamicGPUBuffer<SMInstance>& instanceBuffer,
        const DynamicGPUBuffer<MaterialParameter>& materialBuffer) const;
    
    uint32 GetStructuredBufferSlotIndex(Name name) const;

    const D3D12_ROOT_SIGNATURE_DESC& GetRootSignatureDesc(PassKey<DX12RenderingSubsystem>) const;
    
    virtual bool Recompile(bool immediate = false) override;

    // Asset
public:
    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;
    
    virtual std::vector<std::shared_ptr<Asset>> Import(const std::shared_ptr<Importer>& importer) const override;

    
protected:
    static ComPtr<IDxcResult> CompileShader(const std::filesystem::path& shaderPath,
                                          const std::wstring& entryPoint,
                                          const std::wstring& target,
                                          const std::vector<D3D_SHADER_MACRO>& defines = {});

private:
    struct RecompiledData
    {
        ComPtr<IDxcBlobEncoding> SerializedRootSignature;
        ComPtr<ID3D12RootSignature> RootSignature;
        D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc{};

        ComPtr<ID3D12PipelineState> Pso;

        ComPtr<IDxcBlobEncoding> VertexShader;
        ComPtr<IDxcBlobEncoding> PixelShader;

        std::filesystem::file_time_type LastCompileTime;
        std::unique_ptr<MaterialParameterMap> ParameterMap;
        DArray<StructuredBufferParameter, 4> StructuredBufferParameterTypes;
    };

    DArray<StructuredBufferParameter, 4> _structuredBufferParameters;

    std::atomic<bool> _beingRecompiled = false;
    
    static std::vector<D3D12_INPUT_ELEMENT_DESC> _inputLayout;

    ComPtr<IDxcBlobEncoding> _serializedRootSignature;
    ComPtr<ID3D12RootSignature> _rootSignature;
    D3D12_ROOT_SIGNATURE_DESC _rootSignatureDesc{};

    ComPtr<ID3D12PipelineState> _pso;

    ComPtr<IDxcBlobEncoding> _vertexShader;
    ComPtr<IDxcBlobEncoding> _pixelShader;

    std::filesystem::file_time_type _lastCompileTime;

    uint32 _instanceBufferSlotIndex = 0;
    uint32 _materialBufferSlotIndex = 0;

private:
    bool InitializeRootSignature(const DX12RenderingSubsystem& renderingSubsystem);
    bool InitializePSO(const DX12RenderingSubsystem& renderingSubsystem);

    bool ReflectShaderParameters(IDxcResult* compileResult, std::vector<D3D12_ROOT_PARAMETER>& rootParameters, std::set<MaterialParameterDescriptor>& constantBufferParameterTypes, std::set<StructuredBufferParameter>& structuredBufferParameterTypes) const;
    bool ReflectConstantBuffer(ID3D12ShaderReflection* shaderReflection, const D3D12_SHADER_INPUT_BIND_DESC& bindDesc, std::vector<D3D12_ROOT_PARAMETER>& rootParameters, std::set<MaterialParameterDescriptor>& constantBufferParameterTypes) const;
};

MemoryReader& operator>>(MemoryReader& reader, DX12Shader::StructuredBufferParameter& parameter);
MemoryWriter& operator<<(MemoryWriter& writer, const DX12Shader::StructuredBufferParameter& parameter);
