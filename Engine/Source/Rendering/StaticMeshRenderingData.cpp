#include "StaticMeshRenderingData.h"

void StaticMeshRenderingData::UploadToGPU(RenderingSubsystem& renderingSubsystem)
{
    if (!_isUploaded)
    {
        UploadToGPUInternal(renderingSubsystem);
    }
}

bool StaticMeshRenderingData::IsUploaded() const
{
    return _isUploaded;
}

void StaticMeshRenderingData::SetMesh(const std::shared_ptr<StaticMesh>& mesh, PassKey<StaticMesh>)
{
    _owningMesh = mesh;
    _owningMeshRaw = mesh.get();
}

void StaticMeshRenderingData::PostUpload()
{
    _isUploaded = true;
}

std::shared_ptr<StaticMesh> StaticMeshRenderingData::GetMesh() const
{
    return _owningMesh.lock();
}

StaticMesh& StaticMeshRenderingData::GetMeshRaw() const
{
    return *_owningMeshRaw;
}
