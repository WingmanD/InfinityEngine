#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include "ConstantBuffer.h"
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
    DX12Shader(std::string name, const std::filesystem::path& importPath);

    DX12Shader(const DX12Shader&) = delete;
    DX12Shader(DX12Shader&&) = delete;
    DX12Shader& operator=(const DX12Shader&) = delete;
    DX12Shader& operator=(DX12Shader&&) = delete;

    //void Apply(ID3D12GraphicsCommandList* commandList, PassKey<DX12RenderingSubsystem>);
    void Apply(ID3D12GraphicsCommandList* commandList);

    static std::shared_ptr<DX12Shader> Import(RenderingSubsystem* renderingSubsystem, const std::filesystem::path& path);

    virtual bool Serialize(MemoryWriter& writer) override;
    virtual bool Deserialize(MemoryReader& reader) override;

    const D3D12_ROOT_SIGNATURE_DESC& GetRootSignatureDesc(PassKey<DX12RenderingSubsystem>) const;

protected:
    static ComPtr<ID3DBlob> CompileShader(const std::filesystem::path& shaderPath,
                                          const D3D_SHADER_MACRO* defines,
                                          const std::string& entryPoint,
                                          const std::string& target);

private:
    static std::vector<D3D12_INPUT_ELEMENT_DESC> _inputLayout;

    ComPtr<ID3D12RootSignature> _rootSignature;
    D3D12_ROOT_SIGNATURE_DESC _rootSignatureDesc;

    ComPtr<ID3D12PipelineState> _pso;

    ComPtr<ID3DBlob> _vertexShader;
    ComPtr<ID3DBlob> _pixelShader;
};
