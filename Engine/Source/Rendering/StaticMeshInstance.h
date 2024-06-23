#pragma once

#include "Asset.h"
#include "StaticMesh.h"
#include "Math/Transform.h"
#include "StaticMeshInstance.reflection.h"

REFLECTED(InstancedOnly)
class StaticMeshInstance : public Asset
{
    GENERATED()

public:
    StaticMeshInstance() = default;
    StaticMeshInstance(const SharedObjectPtr<StaticMesh>& mesh, const SharedObjectPtr<Material>& material = nullptr);
    
    [[nodiscard]] SharedObjectPtr<StaticMesh> GetMesh() const;
    void SetMesh(const SharedObjectPtr<StaticMesh>& mesh);
    
    [[nodiscard]] SharedObjectPtr<Material> GetMaterial() const;
    void SetMaterial(const SharedObjectPtr<Material>& material);

private:
    PROPERTY(Edit, Load, Serialize, DisplayName = "Mesh")
    AssetPtr<StaticMesh> _mesh;
    
    PROPERTY(Edit, Load, Serialize, DisplayName = "Material")
    AssetPtr<Material> _material;

    PROPERTY(Edit, Serialize)
    Transform _transform;
};
