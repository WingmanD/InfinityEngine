#pragma once

#include <d3d12.h>
#include <d3d12shader.h>
#include <set>
#include <wrl/client.h>
#include "PassKey.h"
#include "Rendering/Shader.h"
#include "DX12Shader.reflection.h"

struct MaterialParameterDescriptor;
using Microsoft::WRL::ComPtr;

class RenderingSubsystem;
class DX12RenderingSubsystem;

REFLECTED()
class DX12Shader : public Shader
{
    GENERATED()

public:
    DX12Shader() = default;
    DX12Shader(const std::wstring& name);

    DX12Shader(const DX12Shader& other);
    DX12Shader& operator=(const DX12Shader& other);

    //void Apply(ID3D12GraphicsCommandList* commandList, PassKey<DX12RenderingSubsystem>);
    void Apply(ID3D12GraphicsCommandList* commandList) const;

    static std::shared_ptr<DX12Shader> Import(AssetManager& assetManager, const std::filesystem::path& path);

    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

    const D3D12_ROOT_SIGNATURE_DESC& GetRootSignatureDesc(PassKey<DX12RenderingSubsystem>) const;
    
    virtual bool Recompile(bool immediate = false) override;
    
protected:
    static ComPtr<ID3DBlob> CompileShader(const std::filesystem::path& shaderPath,
                                          const D3D_SHADER_MACRO* defines,
                                          const std::string& entryPoint,
                                          const std::string& target);

private:
    struct RecompiledData
    {
        ComPtr<ID3DBlob> SerializedRootSignature;
        ComPtr<ID3D12RootSignature> RootSignature;
        D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc{};

        ComPtr<ID3D12PipelineState> Pso;

        ComPtr<ID3DBlob> VertexShader;
        ComPtr<ID3DBlob> PixelShader;

        std::filesystem::file_time_type LastCompileTime;
        std::unique_ptr<MaterialParameterMap> ParameterMap;
    };

    std::atomic<bool> _beingRecompiled = false;
    
    static std::vector<D3D12_INPUT_ELEMENT_DESC> _inputLayout;

    ComPtr<ID3DBlob> _serializedRootSignature;
    ComPtr<ID3D12RootSignature> _rootSignature;
    D3D12_ROOT_SIGNATURE_DESC _rootSignatureDesc{};

    ComPtr<ID3D12PipelineState> _pso;

    ComPtr<ID3DBlob> _vertexShader;
    ComPtr<ID3DBlob> _pixelShader;

    std::filesystem::file_time_type _lastCompileTime;

private:
    bool InitializeRootSignature(const DX12RenderingSubsystem& renderingSubsystem);
    bool InitializePSO(const DX12RenderingSubsystem& renderingSubsystem);

    bool ReflectShaderParameters(ID3DBlob* shaderBlob, std::vector<D3D12_ROOT_PARAMETER>& rootParameters, std::set<MaterialParameterDescriptor>& constantBufferParameterTypes);
    bool ReflectConstantBuffer(ID3D12ShaderReflection* shaderReflection, const D3D12_SHADER_INPUT_BIND_DESC& bindDesc, std::vector<D3D12_ROOT_PARAMETER>& rootParameters, std::set<MaterialParameterDescriptor>& constantBufferParameterTypes) const;
};
