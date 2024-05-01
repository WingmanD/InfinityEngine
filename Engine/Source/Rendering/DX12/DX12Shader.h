#pragma once

#include "DX12ShaderBase.h"
#include "Importer.h"
#include "PassKey.h"
#include "Rendering/Shader.h"
#include "Containers/DynamicGPUBuffer.h"
#include "Rendering/DX12/StructuredBuffer.h"
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

class DX12Shader : public Shader, public DX12ShaderBase
{
    GENERATED()

public:
    DX12Shader();
    DX12Shader(Name name);

    DX12Shader(const DX12Shader& other);
    DX12Shader& operator=(const DX12Shader& other);

    void Apply(DX12GraphicsCommandList* commandList, PassKey<DX12MaterialRenderingData>) const;
    void BindInstanceBuffers(DX12GraphicsCommandList& commandList,
                             const AppendStructuredBuffer<SMInstance>& instanceBuffer,
                             uint32 instanceStart,
                             const DynamicGPUBuffer<MaterialParameter>& materialBuffer) const;

    // todo temporary
    uint32 GetPointLightsBufferSlotIndex() const
    {
        return _pointLightsBufferSlotIndex;
    }

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
        ShaderReflection Reflection;
    };

    std::atomic<bool> _beingRecompiled = false;

    static std::vector<D3D12_INPUT_ELEMENT_DESC> _inputLayout;

    ComPtr<IDxcBlobEncoding> _vertexShader;
    ComPtr<IDxcBlobEncoding> _pixelShader;

    uint32 _instanceOffsetSlotIndex = 0;
    uint32 _instanceBufferSlotIndex = 0;
    uint32 _materialBufferSlotIndex = 0;
    uint32 _pointLightsBufferSlotIndex = 0;

private:
    bool InitializePSO(const DX12RenderingSubsystem& renderingSubsystem,
                       ComPtr<ID3D12PipelineState>& pso,
                       const ComPtr<ID3D12RootSignature>& rootSignature,
                       const ComPtr<IDxcBlobEncoding>& vertexShader,
                       const ComPtr<IDxcBlobEncoding>& pixelShader);
};
