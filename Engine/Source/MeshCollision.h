#pragma once

#include "Rendering/StaticMesh.h"

class MeshCollision
{
public:
    AssetPtr<StaticMesh> Mesh;
};

MemoryWriter& operator<<(MemoryWriter& writer, const MeshCollision& meshCollision);
MemoryReader& operator>>(MemoryReader& reader, MeshCollision& meshCollision);