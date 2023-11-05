#include "Shader.h"
#include "MaterialParameterMap.h"

Shader::Shader(const std::wstring& name) : Asset(name)
{
}

Shader::Shader(const Shader& other) : Asset(other)
{
}

std::unique_ptr<MaterialParameterMap> Shader::CreateMaterialParameterMap() const
{
    return MaterialParameterMap->Duplicate();
}
