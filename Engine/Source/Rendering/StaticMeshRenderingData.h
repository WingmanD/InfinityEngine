#pragma once

#include "CoreMinimal.h"
#include "NonCopyable.h"
#include "PassKey.h"

class StaticMesh;
class RenderingSubsystem;

class StaticMeshRenderingData : public NonCopyable<StaticMeshRenderingData>
{
public:
    StaticMeshRenderingData() = default;

    virtual ~StaticMeshRenderingData() = default;

    void UploadToGPU(RenderingSubsystem& renderingSubsystem);
    virtual void PostUpload();
    
    bool IsUploaded() const;

    void SetMesh(const std::shared_ptr<StaticMesh>& mesh, uint8 lod, PassKey<StaticMesh>);

protected:
    virtual bool UploadToGPUInternal(RenderingSubsystem& renderingSubsystem) = 0;
    
    std::shared_ptr<StaticMesh> GetMesh() const;

    StaticMesh& GetMeshRaw() const;

    uint8 GetLOD() const;

private:
    std::weak_ptr<StaticMesh> _owningMesh;
    StaticMesh* _owningMeshRaw = nullptr;
    uint8 _lod = 0;
    
    bool _isUploaded = false;
};
