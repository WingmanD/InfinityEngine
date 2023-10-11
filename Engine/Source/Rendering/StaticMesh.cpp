#include "StaticMesh.h"

StaticMesh::StaticMesh(std::string name) : Asset(std::move(name))
{
    Vertex v0;
    v0.Position = {-1.0f, -1.0f, -1.0f};

    Vertex v1;
    v1.Position = {-1.0f, 1.0f, -1.0f};

    Vertex v2;
    v2.Position = {1.0f, 1.0f, -1.0f};

    Vertex v3;
    v3.Position = {1.0f, -1.0f, -1.0f};

    Vertex v4;
    v4.Position = {-1.0f, -1.0f, 1.0f};

    Vertex v5;
    v5.Position = {-1.0f, 1.0f, 1.0f};

    Vertex v6;
    v6.Position = {1.0f, 1.0f, 1.0f};

    Vertex v7;
    v7.Position = {1.0f, -1.0f, 1.0f};


    _vertices = {v0, v1, v2, v3, v4, v5, v6, v7};

    _indices = {
        0, 1, 2,
        0, 2, 3,

        4, 6, 5,
        4, 7, 6,

        4, 5, 1,
        4, 1, 0,

        3, 2, 6,
        3, 6, 7,

        1, 5, 6,
        1, 6, 2,

        4, 0, 3,
        4, 3, 7
    };
}

bool StaticMesh::Serialize(MemoryWriter& writer)
{
    if (!Asset::Serialize(writer))
    {
        return false;
    }

    writer << _vertices;
    writer << _indices;

    return true;
}

bool StaticMesh::Deserialize(MemoryReader& reader)
{
    if (!Asset::Deserialize(reader))
    {
        return false;
    }

    reader >> _vertices;
    reader >> _indices;

    return true;
}

const std::vector<Vertex>& StaticMesh::GetVertices() const
{
    return _vertices;
}

const std::vector<uint32_t>& StaticMesh::GetIndices() const
{
    return _indices;
}

void StaticMesh::SetMaterial(const std::shared_ptr<Material>& material)
{
    _material = material;
}

std::shared_ptr<Material> StaticMesh::GetMaterial() const
{
    return _material;
}
