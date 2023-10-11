#pragma once

#include <d3d12.h>
#include "DX12Shader.h"
#include "Rendering/Material.h"

struct PerPassConstants
{
    DirectX::XMFLOAT4X4 World;
    DirectX::XMFLOAT4X4 ViewProjection;
    float Time = 0.0f;
};

class DX12Material : public Material
{
public:
    DX12Material() = default;
    DX12Material(const std::string& name, const std::shared_ptr<Shader>& shader);

    void Apply(ID3D12GraphicsCommandList* commandList) const;

    PerPassConstants& GetPerPassConstants();

private:
    DX12Shader* _dx12Shader;
    
    ConstantBuffer<PerPassConstants> _perPassConstants;
};
