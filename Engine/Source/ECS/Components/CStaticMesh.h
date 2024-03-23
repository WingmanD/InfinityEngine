#pragma once

#include "Component.h"
#include "AssetPtr.h"
#include "Rendering/StaticMesh.h"
#include "CStaticMesh.reflection.h"

REFLECTED()
class CStaticMesh : public Component
{
    GENERATED()

public:
    PROPERTY(Edit, Serialize)
    AssetPtr<StaticMesh> Mesh;
    // todo transform
};
