#pragma once

#include "Rendering/MaterialRenderingData.h"
#include "Rendering/DX12/DX12RenderingCore.h"

class DX12MaterialRenderingData : public MaterialRenderingData
{
public:
    void Apply(DX12GraphicsCommandList* commandList) const;    
};
