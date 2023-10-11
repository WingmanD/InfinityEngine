#pragma once

#include "Asset.h"
#include "Material.h"
#include "StaticMesh.reflection.h"

// struct Vertex
// {
//     Vector3 Position;
//     Vector3 Normal;
//     Vector3 Tangent;
//     Vector3 Bitangent;
//     Vector3 Color;
//     Vector3 UV;
// };

struct Vertex
{
    Vector3 Position;
    Vector3 Color;
};

REFLECTED()
class StaticMesh : public Asset
{
    STATICMESH_GENERATED()

public:
    StaticMesh() = default;

    explicit StaticMesh(std::string name);

    virtual bool Serialize(MemoryWriter& writer) override;

    virtual bool Deserialize(MemoryReader& reader) override;

    [[nodiscard]] const std::vector<Vertex>& GetVertices() const;

    [[nodiscard]] const std::vector<uint32_t>& GetIndices() const;

    void SetMaterial(const std::shared_ptr<Material>& material);
    [[nodiscard]] std::shared_ptr<Material> GetMaterial() const;

private:
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;

    std::shared_ptr<Material> _material;
};
