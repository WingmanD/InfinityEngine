#pragma once

#include "Rendering/DX12/DX12ShaderBase.h"
#include "Rendering/Shader.h"
#include "DX12ComputeShader.reflection.h"

REFLECTED(CustomSerialization)
class DX12ComputeShader : public Shader, public DX12ShaderBase
{
    GENERATED()
    
public:
    virtual bool Recompile(bool immediate = false) override;

    virtual void Dispatch(DX12GraphicsCommandList& commandList, uint32 threadGroupCountX, uint32 threadGroupCountY, uint32 threadGroupCountZ) const;

    // Shader
public:
    virtual bool Initialize() override;

    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

protected:
    virtual void BindResources(DX12GraphicsCommandList& commandList) const;
    
    ID3D12PipelineState* GetPSO() const;
    ID3D12RootSignature* GetRootSignature() const;
    
private:
    ComPtr<ID3D12PipelineState> _pso;
    ComPtr<ID3D12RootSignature> _rootSignature;
    ComPtr<IDxcBlobEncoding> _serializedRootSignature;
    ComPtr<IDxcBlobEncoding> _computeShader;
};
