#include "Shader.h"
#include "MaterialParameterMap.h"

Shader::Shader(Name name) : Asset(name)
{
}

Shader::Shader(const Shader& other) : Asset(other)
{
}

bool Shader::Recompile(bool immediate)
{
    return true;
}

std::unique_ptr<MaterialParameterMap> Shader::CreateMaterialParameterMap() const
{
    if (ParameterMap == nullptr)
    {
        return nullptr;
    }
    
    return ParameterMap->Duplicate();
}
