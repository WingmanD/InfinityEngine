#include "Material.h"
#include "PropertyMap.h"

Material::Material(const std::string& name, const std::shared_ptr<Shader>& shader) : Asset(name), _shader(shader)
{
    
}

std::shared_ptr<Shader> Material::GetShader() const
{
    return _shader;
}
