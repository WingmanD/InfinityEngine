#pragma once

#include "Asset.h"
#include "Material.h"
#include "StaticMeshRenderingData.h"
#include "StaticMesh.reflection.h"

struct aiMesh;

struct Vertex
{
    Vector3 Position;
    Vector3 Normal;
    Vector4 Color;
    Vector2 UV;
};

REFLECTED()
class StaticMesh : public Asset
{
    STATICMESH_GENERATED()

public:
    StaticMesh() = default;
    
    StaticMesh(const StaticMesh& other);
    StaticMesh& operator=(const StaticMesh& other);
    
    explicit StaticMesh(std::wstring name);

    virtual bool Initialize() override;

    virtual bool Serialize(MemoryWriter& writer) const override;

    virtual bool Deserialize(MemoryReader& reader) override;

    static std::vector<std::shared_ptr<StaticMesh>> BatchImport(const std::filesystem::path& path);

    [[nodiscard]] const std::vector<Vertex>& GetVertices() const;

    [[nodiscard]] const std::vector<uint32_t>& GetIndices() const;

    // todo multiple material slots
    void SetMaterial(const std::shared_ptr<Material>& material);
    [[nodiscard]] std::shared_ptr<Material> GetMaterial() const;

    StaticMeshRenderingData* GetRenderingData() const;

private:
    bool ImportInternal(const aiMesh* assimpMesh);

private:
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;

    PROPERTY(Load)
    std::shared_ptr<Material> _material;

    std::unique_ptr<StaticMeshRenderingData> _renderingData;
};
