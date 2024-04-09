#pragma once

#include "ConstantBuffer.h"
#include "Rendering/MaterialParameterRenderingData.h"

class DX12MaterialParameterRenderingData : public MaterialParameterRenderingData
{
public:
    virtual bool Initialize(MaterialParameter* parameter) override;
    
    ConstantBuffer<MaterialParameter>& GetConstantBuffer();
    
private:
    ConstantBuffer<MaterialParameter> _constantBuffer{};
};
