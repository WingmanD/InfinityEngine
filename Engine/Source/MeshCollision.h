#pragma once

#include "Rendering/StaticMesh.h"

class MeshCollision
{
public:
    AssetPtr<StaticMesh> Mesh;

    Vector3 FurthestPointInDirection(const Vector3& direction) const;
};

MemoryWriter& operator<<(MemoryWriter& writer, const MeshCollision& meshCollision);
MemoryReader& operator>>(MemoryReader& reader, MeshCollision& meshCollision);