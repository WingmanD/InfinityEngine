#include "StaticMeshInstance.h"
#include "StaticMesh.h"

StaticMeshInstance::StaticMeshInstance(const SharedObjectPtr<StaticMesh>& mesh, const SharedObjectPtr<Material>& material) : Asset(), _mesh(mesh), _material(material)
{
    if (material == nullptr && mesh != nullptr)
    {
        _material = mesh->GetMaterial();
    }
}

SharedObjectPtr<StaticMesh> StaticMeshInstance::GetMesh() const
{
    return _mesh;
}

void StaticMeshInstance::SetMesh(const SharedObjectPtr<StaticMesh>& mesh)
{
    _mesh = mesh;
}

SharedObjectPtr<Material> StaticMeshInstance::GetMaterial() const
{
    return _material;
}

void StaticMeshInstance::SetMaterial(const SharedObjectPtr<Material>& material)
{
    _material = material;
}
