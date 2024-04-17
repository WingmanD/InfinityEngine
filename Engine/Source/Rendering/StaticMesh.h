#pragma once

#include "Asset.h"
#include "AssetPtr.h"
#include "Importer.h"
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
class StaticMeshImporter : public Importer
{
    GENERATED()

public:
    PROPERTY(Edit)
    std::filesystem::path Path;

public:
    StaticMeshImporter() = default;
};

REFLECTED(CustomSerialization)
class StaticMesh : public Asset
{
    GENERATED()

public:
    static std::shared_ptr<StaticMesh> GetMeshByID(uint32 meshID);
    
public:
    StaticMesh();
    
    StaticMesh(const StaticMesh& other);
    StaticMesh& operator=(const StaticMesh& other);

    explicit StaticMesh(Name name);

    virtual bool Initialize() override;

    virtual bool Serialize(MemoryWriter& writer) const override;

    virtual bool Deserialize(MemoryReader& reader) override;

    [[nodiscard]] const std::vector<Vertex>& GetVertices() const;

    [[nodiscard]] const std::vector<uint32_t>& GetIndices() const;

    // todo multiple material slots
    void SetMaterial(const std::shared_ptr<Material>& material);
    [[nodiscard]] std::shared_ptr<Material> GetMaterial() const;

    uint32 GetMeshID() const;

    StaticMeshRenderingData* GetRenderingData() const;

    template <typename T> requires IsA<T, StaticMeshRenderingData>
    T* GetRenderingData() const
    {
        return static_cast<T*>(_renderingData.get());
    }

    // Asset
public:
    virtual std::vector<std::shared_ptr<Asset>> Import(const std::shared_ptr<Importer>& importer) const override;

private:
    bool ImportInternal(const aiMesh* assimpMesh);

private:
    static IDGenerator<uint32> _meshIDGenerator;
    static std::unordered_map<uint32, std::weak_ptr<StaticMesh>> _meshIDToStaticMesh;

private:
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;

    PROPERTY(Edit, Load)
    AssetPtr<Material> _material;

    uint32 _meshID = 0;

    std::unique_ptr<StaticMeshRenderingData> _renderingData;
};
