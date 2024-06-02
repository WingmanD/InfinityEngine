#pragma once

#include "Asset.h"
#include "AssetPtr.h"
#include "BoundingBox.h"
#include "Importer.h"
#include "Material.h"
#include "StaticMeshRenderingData.h"
#include "StaticMesh.reflection.h"
#include "Containers/DynamicGPUBuffer2.h"

struct aiMesh;

struct Vertex
{
    Vector3 Position;
    Vector3 Normal;
    Vector3 Tangent;
    Vector3 Bitangent;
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
    struct MeshInfo
    {
        Vector3 AABBMin;
        Vector3 AABBMax;
    };

    struct LOD
    {
    public:
        DArray<Vertex> Vertices;
        DArray<uint32> Indices;
        std::unique_ptr<StaticMeshRenderingData> RenderingData = nullptr;

        // todo we need materials here, multiple slots per lod, but we need struct reflection for that

    public:
        LOD() = default;
        
        LOD(const LOD& other);
        LOD& operator=(const LOD& other);

        LOD(LOD&& other) noexcept;
        LOD& operator=(LOD&& other) noexcept;

        ~LOD() = default;

        template <typename T> requires IsA<T, StaticMeshRenderingData>
        T* GetRenderingData() const
        {
            return static_cast<T*>(RenderingData.get());
        }
    };
    
public:
    static std::shared_ptr<StaticMesh> GetMeshByID(uint32 meshID);
    static DynamicGPUBuffer2<MeshInfo>& GetMeshInfoBuffer();
    
public:
    StaticMesh();
    
    StaticMesh(const StaticMesh& other);
    StaticMesh& operator=(const StaticMesh& other);

    explicit StaticMesh(Name name);

    virtual bool Initialize() override;

    virtual bool Serialize(MemoryWriter& writer) const override;

    virtual bool Deserialize(MemoryReader& reader) override;

    const LOD& GetLOD(uint8 lod) const;

    // todo multiple material slots
    void SetMaterial(const std::shared_ptr<Material>& material);
    [[nodiscard]] std::shared_ptr<Material> GetMaterial() const;

    uint32 GetMeshID() const;

    const BoundingBox& GetBoundingBox() const;

    // Asset
public:
    virtual DArray<std::shared_ptr<Asset>> Import(const std::shared_ptr<Importer>& importer) const override;

    // Asset
protected:
    virtual void PostLoad() override;

private:
    static IDGenerator<uint32> _meshIDGenerator;
    static std::unordered_map<uint32, std::weak_ptr<StaticMesh>> _meshIDToStaticMesh;
    static DynamicGPUBuffer2<MeshInfo> _meshInfoBuffer;

private:
    DArray<LOD, 10> _lods;

    PROPERTY(Edit, Load)
    AssetPtr<Material> _material;

    uint32 _meshID = 0;
    
    BoundingBox _boundingBox;

private:
    bool ImportLOD(const aiMesh* assimpMesh, uint8 lodIndex);
    void UpdateBoundingBox();
};

MemoryWriter& operator<<(MemoryWriter& writer, const StaticMesh::LOD& lod);
MemoryReader& operator>>(MemoryReader& reader, StaticMesh::LOD& lod);
