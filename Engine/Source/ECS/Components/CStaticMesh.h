#pragma once

#include "Component.h"
#include "AssetPtr.h"
#include "Math/Transform.h"
#include "Rendering/StaticMesh.h"
#include "CStaticMesh.reflection.h"

REFLECTED()
class CStaticMesh : public Component
{
    GENERATED()

public:
    PROPERTY(Edit, Serialize)
    AssetPtr<StaticMesh> Mesh;

    PROPERTY(Edit, Serialize)
    AssetPtr<Material> MaterialOverride;

    PROPERTY(Edit, Serialize)
    Transform MeshTransform;

    uint32 InstanceID = 0;
};
