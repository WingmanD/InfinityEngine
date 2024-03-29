﻿#pragma once

#include "Asset.h"
#include "Delegate.h"
#include "MaterialParameterMap.h"
#include "Shader.reflection.h"

REFLECTED()
class Shader : public Asset
{
    GENERATED()

public:
    Delegate<const Shader*> OnRecompiled;

public:
    Shader() = default;
    Shader(Name name);

    Shader(const Shader& other);

    virtual bool Recompile(bool immediate = false);

    std::unique_ptr<MaterialParameterMap> CreateMaterialParameterMap() const;

protected:
    std::unique_ptr<MaterialParameterMap> ParameterMap;
};
