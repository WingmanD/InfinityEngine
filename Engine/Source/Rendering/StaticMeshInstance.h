#pragma once

#include "Asset.h"
#include "StaticMeshInstance.reflection.h"

class Material;
class StaticMesh;

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
    PROPERTY(EditableInEditor, Load, DisplayName = "Mesh")
    std::shared_ptr<StaticMesh> _mesh;
    
    PROPERTY(EditableInEditor, Load, DisplayName = "Material")
    std::shared_ptr<Material> _material;
};
