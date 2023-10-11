#pragma once

#include "Asset.h"
#include "Shader.reflection.h"

REFLECTED()
class Shader : public Asset
{
    SHADER_GENERATED()
    
public:
    Shader() = default;
    
    Shader(const std::string& name, std::filesystem::path::iterator::reference path);
};
