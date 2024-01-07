#pragma once

#include "Asset.h"
#include "StaticMesh.h"
#include "StaticMeshInstance.reflection.h"

REFLECTED(InstancedOnly)
class StaticMeshInstance : public Asset
{
    GENERATED()

public:
    StaticMeshInstance() = default;
    StaticMeshInstance(const std::shared_ptr<StaticMesh>& mesh, const std::shared_ptr<Material>& material = nullptr);
    
    [[nodiscard]] std::shared_ptr<StaticMesh> GetMesh() const;
    void SetMesh(const std::shared_ptr<StaticMesh>& mesh);
    
    [[nodiscard]] std::shared_ptr<Material> GetMaterial() const;
    void SetMaterial(const std::shared_ptr<Material>& material);

private:
    PROPERTY(Edit, Load, DisplayName = "Mesh")
    AssetPtr<StaticMesh> _mesh;
    
    PROPERTY(Edit, Load, DisplayName = "Material")
    AssetPtr<Material> _material;
};
