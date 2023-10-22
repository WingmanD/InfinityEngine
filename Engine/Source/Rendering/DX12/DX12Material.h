#pragma once

#include "ConstantBuffer.h"
#include "Rendering/Material.h"
#include <d3d12.h>
#include "DX12Material.reflection.h"

struct PerPassConstants
{
    DirectX::XMFLOAT4X4 World;
    DirectX::XMFLOAT4X4 ViewProjection;
    Vector3 CameraPosition;
    Vector3 CameraDirection;
    float Time = 0.0f;
};

REFLECTED()
class DX12Material : public Material
{
    DX12MATERIAL_GENERATED()
    
public:
    DX12Material() = default;
    DX12Material(const std::wstring& name);

    virtual bool Initialize() override;
    
    void Apply(ID3D12GraphicsCommandList* commandList) const;

    PerPassConstants& GetPerPassConstants();

private:
    ConstantBuffer<PerPassConstants> _perPassConstants;
};
