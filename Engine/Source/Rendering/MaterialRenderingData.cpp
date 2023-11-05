#include "MaterialRenderingData.h"

void MaterialRenderingData::SetMaterial(Material* material, PassKey<Material>)
{
    _material = material;
}

Material& MaterialRenderingData::GetMaterial() const
{
    return *_material;
}
