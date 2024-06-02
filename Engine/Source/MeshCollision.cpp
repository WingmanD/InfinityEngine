#include "MeshCollision.h"

Vector3 MeshCollision::FurthestPointInDirection(const Vector3& direction) const
{
    if (Mesh == nullptr)
    {
        return {};
    }

    Vector3 furthestPoint;
    float maxDistance = -std::numeric_limits<float>::max();

    const DArray<Vertex>& vertices = Mesh->GetLOD(0).Vertices;
    for (uint32 i = 0; i < vertices.Count(); ++i)
    {
        const Vertex& vertex = vertices[i];

        float distance = vertex.Position.Dot(direction);
        if (distance > maxDistance)
        {
            maxDistance = distance;
            furthestPoint = vertex.Position;
        }
    }

    return furthestPoint;
}

MemoryWriter& operator<<(MemoryWriter& writer, const MeshCollision& meshCollision)
{
    writer << meshCollision.Mesh;
    return writer;
}

MemoryReader& operator>>(MemoryReader& reader, MeshCollision& meshCollision)
{
    reader >> meshCollision.Mesh;
    return reader;
}
