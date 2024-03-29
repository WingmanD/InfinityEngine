﻿#pragma once

#include "NonCopyable.h"
#include "PassKey.h"
#include <memory>

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

    void SetMesh(const std::shared_ptr<StaticMesh>& mesh, PassKey<StaticMesh>);

protected:
    virtual bool UploadToGPUInternal(RenderingSubsystem& renderingSubsystem) = 0;
    
    std::shared_ptr<StaticMesh> GetMesh() const;

    StaticMesh& GetMeshRaw() const;

private:
    std::weak_ptr<StaticMesh> _owningMesh;
    StaticMesh* _owningMeshRaw = nullptr;
    
    bool _isUploaded = false;
};
