#pragma once

#include "Asset.h"
#include "Containers/SparseUniformGrid3D.h"
#include "Math/Transform.h"
#include "Rendering/Widgets/AssetBrowser.h"
#include "Level.reflection.h"

class EntityTemplate;

REFLECTED()
class LevelAssetBrowserEntry : public AssetBrowserEntry
{
    GENERATED()

public:
    LevelAssetBrowserEntry() = default;

    // AssetBrowserEntry
public:
    virtual bool InitializeFromAsset(const SharedObjectPtr<Asset>& asset) override;
};

REFLECTED(CustomSerialization)
class Level : public Asset
{
    GENERATED()

public:
    struct EntityElement
    {
        uint64 EntityTemplateID = 0;
        uint64 ID = 0;
        Transform EntityTransform;
    };

    struct Chunk
    {
        DArray<EntityElement> EntityElements;

        std::atomic<bool> IsLoaded = false;
        std::atomic<bool> IsBeingLoaded = false;
        
        uint64 Offset = 0;
        uint64 ByteSize = 0;
    };

public:
    Level() = default;

    Level(const Level& other);
    
    uint64 AddEntity(const SharedObjectPtr<EntityTemplate>& entityTemplate, const Transform& transform);
    void RemoveEntity(uint64 elementID);

    uint64 MoveEntity(uint64 elementID, const Transform& transform);

    void Stream(const Vector3& location, float radius, const std::function<void(const Chunk& chunk)>& callback, bool includeLoaded = false);
    void LoadAllChunks();

    void ForEachChunk(const std::function<void(Chunk& chunk)>& callback);
    void ForEachChunk(const std::function<void(const Chunk& chunk)>& callback) const;

    void InitializeEdit();

    // Asset
public:
    virtual bool SaveInternal(std::ofstream& file) const override;
    
    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

    virtual SharedObjectPtr<Widget> CreateEditWidget() override;

    // Object

private:
    struct EntityElementRef
    {
        Chunk* Chunk;
        uint64 EntityIndex = 0;
    };
    
    static constexpr float _chunkDimension = 20.0f;
    
    SparseUniformGrid3D<Chunk> _grid;

    std::unordered_map<uint64, EntityElementRef> _entityElementRefs;

    bool _isFullyLoaded = false;
    std::atomic<uint32> _chunksLoaded = 0;
    uint64 _chunksOffset = 0;

    IDGenerator<uint64> _elementIDGenerator;

private:
    static Index3D GetChunkIndex(const Vector3& location);
    
    void LoadChunk(Chunk& chunk);
    Chunk& GetChunkAt(const Vector3& location);
};

MemoryReader& operator>>(MemoryReader& reader, Level::EntityElement& element);
MemoryWriter& operator<<(MemoryWriter& writer, const Level::EntityElement& element);

MemoryReader& operator>>(MemoryReader& reader, Level::Chunk& chunk);
MemoryWriter& operator<<(MemoryWriter& writer, const Level::Chunk& chunk);
