#include "StaticMeshInstance.h"
#include "StaticMesh.h"

StaticMeshInstance::StaticMeshInstance(const std::shared_ptr<StaticMesh>& mesh, const std::shared_ptr<Material>& material) : Asset(), _mesh(mesh), _material(material)
{
    if (material == nullptr && mesh != nullptr)
    {
        _material = mesh->GetMaterial();
    }
}

std::shared_ptr<StaticMesh> StaticMeshInstance::GetMesh() const
{
    return _mesh;
}

void StaticMeshInstance::SetMesh(const std::shared_ptr<StaticMesh>& mesh)
{
    _mesh = mesh;
}

std::shared_ptr<Material> StaticMeshInstance::GetMaterial() const
{
    return _material;
}

void StaticMeshInstance::SetMaterial(const std::shared_ptr<Material>& material)
{
    _material = material;
}
