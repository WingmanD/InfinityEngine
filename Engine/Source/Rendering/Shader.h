#pragma once

#include "Asset.h"
#include "Shader.reflection.h"

class MaterialParameterMap;

REFLECTED()
class Shader : public Asset
{
    SHADER_GENERATED()
    
public:
    Shader() = default;
    Shader(const std::wstring& name);

    Shader(const Shader& other);

    std::unique_ptr<MaterialParameterMap> CreateMaterialParameterMap() const;

protected:
    std::unique_ptr<MaterialParameterMap> MaterialParameterMap;
};
