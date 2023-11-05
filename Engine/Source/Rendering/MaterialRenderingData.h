#pragma once

#include "PassKey.h"

class Material;

class MaterialRenderingData
{
public:
    MaterialRenderingData() = default;
    virtual ~MaterialRenderingData() = default;
    
    void SetMaterial(Material* material, PassKey<Material>);

protected:
    Material& GetMaterial() const;

private:
    Material* _material = nullptr;
};
