#pragma once

#include "Rendering/DX12/DX12RenderingCore.h"
#include "Rendering/MaterialParameterMap.h"

class DX12MaterialParameterMap : public MaterialParameterMap
{
public:
    DX12MaterialParameterMap() = default;

    void Bind(DX12GraphicsCommandList* commandList) const;
};
