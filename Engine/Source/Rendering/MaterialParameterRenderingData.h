#pragma once

#include "NonCopyable.h"

struct MaterialParameter;

class MaterialParameterRenderingData : public NonCopyable<MaterialParameterRenderingData>
{
public:
    virtual ~MaterialParameterRenderingData() = default;
    
    virtual bool Initialize(MaterialParameter* parameter) = 0;
};
