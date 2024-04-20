#pragma once

#include "Rendering/DX12/DX12ShaderBase.h"
#include "Rendering/Shader.h"
#include "DX12ComputeShader.reflection.h"

REFLECTED()
class DX12ComputeShader : public Shader, public DX12ShaderBase
{
    GENERATED()
    
public:
    bool Recompile(bool immediate = false) override;
private:
};
