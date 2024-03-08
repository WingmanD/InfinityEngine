#pragma once

#include "AssetPtr.h"
#include "Component.h"
#include "Rendering/StaticMesh.h"
#include "CStaticMesh.reflection.h"

REFLECTED()
class CStaticMesh : public Component
{
    GENERATED()

public:
    AssetPtr<StaticMesh> Mesh;
    // todo transform

public:
    CStaticMesh() = default;
};
