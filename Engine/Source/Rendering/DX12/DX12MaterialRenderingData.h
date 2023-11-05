#pragma once

#include <d3d12.h>

#include "Rendering/MaterialRenderingData.h"

class DX12MaterialRenderingData : public MaterialRenderingData
{
public:
    void Apply(ID3D12GraphicsCommandList* commandList) const;    
};
