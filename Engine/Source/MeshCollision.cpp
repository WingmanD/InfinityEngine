#include "MeshCollision.h"

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
