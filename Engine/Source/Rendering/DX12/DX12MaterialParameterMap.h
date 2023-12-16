#pragma once

#include "Rendering/MaterialParameterMap.h"
#include "ConstantBuffer.h"

class DX12MaterialParameterMap : public MaterialParameterMap
{
public:
    DX12MaterialParameterMap() = default;

    void Bind(ID3D12GraphicsCommandList* commandList) const;
};
