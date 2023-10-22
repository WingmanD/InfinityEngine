#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include "PassKey.h"
#include "Rendering/Shader.h"
#include "DX12Shader.reflection.h"

using Microsoft::WRL::ComPtr;

class RenderingSubsystem;
class DX12RenderingSubsystem;

REFLECTED()
class DX12Shader : public Shader
{
    DX12SHADER_GENERATED()

public:
    DX12Shader() = default;
    DX12Shader(const std::wstring& name);

    DX12Shader(const DX12Shader& other);
    DX12Shader& operator=(const DX12Shader& other);

    virtual bool Initialize() override;

    //void Apply(ID3D12GraphicsCommandList* commandList, PassKey<DX12RenderingSubsystem>);
    void Apply(ID3D12GraphicsCommandList* commandList);

    static std::shared_ptr<DX12Shader> Import(AssetManager& assetManager, const std::filesystem::path& path);

    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

    const D3D12_ROOT_SIGNATURE_DESC& GetRootSignatureDesc(PassKey<DX12RenderingSubsystem>) const;

protected:
    bool Compile();
    
    static ComPtr<ID3DBlob> CompileShader(const std::filesystem::path& shaderPath,
                                          const D3D_SHADER_MACRO* defines,
                                          const std::string& entryPoint,
                                          const std::string& target);

private:
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
    void InitializePSO(const DX12RenderingSubsystem& renderingSubsystem);
};
