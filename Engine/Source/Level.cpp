#include "Level.h"
#include "LevelEditorGame.h"
#include "ProjectSettings.h"
#include "ECS/Systems/LevelEditorSystem.h"
#include "Engine/Engine.h"
#include "Rendering/Widgets/Button.h"
#include "Rendering/Widgets/FlowBox.h"
#include "Rendering/Widgets/TextBox.h"
#include "Rendering/Widgets/ViewportWidget.h"
#include <ranges>

bool LevelAssetBrowserEntry::InitializeFromAsset(const SharedObjectPtr<Asset>& asset)
{
    if (!AssetBrowserEntry::InitializeFromAsset(asset))
    {
        return false;
    }

    const SharedObjectPtr<Button> spawnButton = AddChild<Button>();
    if (spawnButton == nullptr)
    {
        return false;
    }
    spawnButton->SetText(L"Spawn");
    spawnButton->GetTextBox()->SetPadding({4.0f, 4.0f, 1.0f, 1.0f});
    std::ignore = spawnButton->OnReleased.Add([asset, this]()
    {
        if (!asset->Load())
        {
            LOG(L"Failed to load asset for spawning: {}", asset->GetName().ToString());
            return;
        }

        GameplaySubsystem::Get().GetWorlds().ForEach([asset](World& world)
        {
            world.FindSystem<LevelEditorSystem>()->SpawnEntity(std::dynamic_pointer_cast<EntityTemplate>(asset));

            return true;
        });
    });

    return true;
}

Level::Level(const Level& other) : Asset(other)
{
    // todo
}

uint64 Level::AddEntity(const SharedObjectPtr<EntityTemplate>& entityTemplate, const Transform& transform)
{
    const uint64 id = _elementIDGenerator.GenerateID();
    
    Chunk& chunk = GetChunkAt(transform.GetWorldLocation());
    
    EntityElement newElement;
    newElement.EntityTemplateID = entityTemplate->GetAssetID();
    newElement.ID = id;
    newElement.EntityTransform = transform;
    chunk.EntityElements.Emplace(newElement);

    _entityElementRefs[id] = {&chunk, chunk.EntityElements.Count() - 1};

    return id; 
}

void Level::RemoveEntity(uint64 elementID)
{
    EntityElementRef& elementRef = _entityElementRefs[elementID];
    
    elementRef.Chunk->EntityElements.RemoveAtSwap(elementRef.EntityIndex);
    _entityElementRefs.erase(elementRef.Chunk->EntityElements[elementRef.EntityIndex].ID);
    
    if (!elementRef.Chunk->EntityElements.IsEmpty())
    {
        _entityElementRefs[elementRef.Chunk->EntityElements[elementRef.EntityIndex].ID].EntityIndex = elementRef.EntityIndex;
    }
}

void Level::Stream(const Vector3& location, float radius, const std::function<void(const Chunk& chunk)>& callback, bool includeLoaded /*= false*/)
{
    const Vector3 min = location - Vector3(radius);
    const Vector3 max = location + Vector3(radius);

    _grid.ForEachCellInBox(
        GetChunkIndex(min),
        GetChunkIndex(max),
        [this, &callback, includeLoaded](const Index3D& index, Chunk& chunk)
        {
            if (chunk.IsLoaded)
            {
                if (includeLoaded)
                {
                    callback(chunk);
                }
                
                return true;
            }

            if (chunk.IsBeingLoaded)
            {
                return true;
            }
            
            chunk.IsBeingLoaded = true;

            Engine::Get().GetWaitThreadPool().EnqueueTask([this, &chunk, callback]()
            {
                LoadChunk(chunk);
                callback(chunk);
            });

            return true;
        });
}

void Level::LoadAllChunks()
{
    if (!Load())
    {
        LOG(L"ERROR: Failed to load level {}!", GetName());
        return;
    }
    
    if (_isFullyLoaded)
    {
        return;
    }

    if (_chunksLoaded == _grid.Count())
    {
        _isFullyLoaded = true;
        return;
    }
    
    for (Chunk& chunk : _grid | std::views::values)
    {
        if (chunk.IsLoaded)
        {
            continue;
        }

        if (chunk.IsBeingLoaded)
        {
            continue;
        }

        chunk.IsBeingLoaded = true;

        LoadChunk(chunk);
    }
}

void Level::ForEachChunk(const std::function<void(Chunk& chunk)>& callback)
{
    for (Chunk& chunk : _grid | std::views::values)
    {
        callback(chunk);
    }
}

void Level::ForEachChunk(const std::function<void(const Chunk& chunk)>& callback) const
{
    for (const Chunk& chunk : _grid | std::views::values)
    {
        callback(chunk);
    }
}

void Level::InitializeEdit()
{
    for (Chunk& chunk : _grid | std::views::values)
    {
        for (uint64 i = 0; i < chunk.EntityElements.Count(); ++i)
        {
            EntityElement& element = chunk.EntityElements[i];
            _entityElementRefs[element.ID] = {&chunk, i};
        }
    }
}

bool Level::SaveInternal(std::ofstream& file) const
{
    const_cast<Level*>(this)->LoadAllChunks();

    if (!Asset::SaveInternal(file))
    {
        return false;
    }

    MemoryWriter writer;
    for (const Chunk& chunk : _grid | std::views::values)
    {
        writer << chunk;
    }

    if (!writer.WriteToFile(file))
    {
        LOG(L"Failed to write level {} to file {}!", GetName(), GetAssetPath().wstring());
        return false;
    }

    return true;
}

bool Level::Serialize(MemoryWriter& writer) const
{
    if (!Asset::Serialize(writer))
    {
        return false;
    }
    
    writer << _grid.Count();

    uint64 chunkOffset = 0;
    for (auto& [index, chunk] : _grid)
    {
        writer << index;
        writer << chunkOffset;

        chunkOffset += chunk.EntityElements.Count() * sizeof(uint64) * 2 + sizeof(Vector3) * 3 + sizeof(uint64);
    }
    
    writer << _elementIDGenerator;

    writer << writer.GetByteCount() + sizeof(uint64);

    return true;
}

bool Level::Deserialize(MemoryReader& reader)
{
    if (!Asset::Deserialize(reader))
    {
        return false;
    }

    uint64 chunkCount = 0;
    reader >> chunkCount;

    Chunk* previous = nullptr;
    for (uint64 i = 0; i < chunkCount; ++i)
    {
        uint64 index;
        reader >> index;

        reader >> _grid[index].Offset;
        if (previous != nullptr)
        {
            previous->ByteSize = _grid[index].Offset - previous->Offset;
        }

        previous = &_grid[index];
    }

    reader >> _elementIDGenerator;
    
    reader >> _chunksOffset;

    if (reader.GetNumRemainingBytes() != 0)
    {
        DEBUG_BREAK();
    }
    
    return true;
}

SharedObjectPtr<Widget> Level::CreateEditWidget()
{
    SharedObjectPtr<FlowBox> flowBox = NewObject<FlowBox>();
    if (!flowBox->Initialize())
    {
        return nullptr;
    }
    flowBox->SetDirection(EFlowBoxDirection::Horizontal);
    flowBox->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);

    SharedObjectPtr<AssetBrowser> browser = NewObject<AssetBrowser>();

    browser->SetEntryType(LevelAssetBrowserEntry::StaticType());
    browser->SetFilter([](const Asset& asset)
    {
        return dynamic_cast<const EntityTemplate*>(&asset) != nullptr;
    });
    if (!browser->Initialize())
    {
        return nullptr;
    }
    flowBox->AddChild(browser);
    
    SharedObjectPtr<ViewportWidget> viewport = flowBox->AddChild<ViewportWidget>();
    if (!viewport->Initialize())
    {
        return nullptr;
    }

    viewport->SetCaptureMouseOnClick(false);
    viewport->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);

    GameplaySubsystem& gameplaySubsystem = GameplaySubsystem::Get();
    const SharedObjectPtr<ProjectSettings> projectSettings = ProjectSettings::Get();

    const SharedObjectPtr<LevelEditorGame> game = projectSettings->GetLevelEditorGame()->DuplicateObject<LevelEditorGame>();

    game->SetLevel(SharedFromThis());
    
    gameplaySubsystem.StartGame(viewport, game);

    std::ignore = viewport->OnDestroyed.Add([&gameplaySubsystem]()
    {
       gameplaySubsystem.StopGame(); 
    });

    return flowBox;
}

Index3D Level::GetChunkIndex(const Vector3& location)
{
    Index3D index;
    index.X = static_cast<int32>(location.x / _chunkDimension);
    index.Y = static_cast<int32>(location.y / _chunkDimension);
    index.Z = static_cast<int32>(location.z / _chunkDimension);

    return index;
}

void Level::LoadChunk(Chunk& chunk)
{
    if (chunk.IsLoaded)
    {
        return;
    }

    std::ifstream file(GetAssetPath(), std::ios::binary);
    if (!file.is_open())
    {
        LOG(L"Failed to open file {} for reading (level {})!", GetAssetPath().wstring(), GetName());
        return;
    }
    file.seekg(_chunksOffset + sizeof(uint64));

    MemoryReader reader;
    reader.ReadFromFile(file, chunk.Offset, chunk.ByteSize);

    reader >> chunk;
    
    AssetManager& assetManager = AssetManager::Get();
    for (const EntityElement& entityElement : chunk.EntityElements)
    {
        SharedObjectPtr<EntityTemplate> entityTemplate = assetManager.FindAsset<EntityTemplate>(entityElement.EntityTemplateID);
        if (entityTemplate == nullptr)
        {
            continue;
        }

        entityTemplate->Load();
    } 

    chunk.IsBeingLoaded = false;
    chunk.IsLoaded = true;
    ++_chunksLoaded;
}

Level::Chunk& Level::GetChunkAt(const Vector3& location)
{
    const Index3D index = GetChunkIndex(location);
    Chunk* chunk = _grid.GetIfExists(index.X, index.Y, index.Z);
    if (chunk != nullptr)
    {
        return *chunk;
    }

    Chunk& newChunk = _grid[index];
    newChunk.IsLoaded = true;
    
    return newChunk;
}

MemoryReader& operator>>(MemoryReader& reader, Level::EntityElement& element)
{
    reader >> element.EntityTemplateID;
    reader >> element.ID;
    reader >> element.EntityTransform;

    return reader;
}

MemoryWriter& operator<<(MemoryWriter& writer, const Level::EntityElement& element)
{
    writer << element.EntityTemplateID;
    writer << element.ID;
    writer << element.EntityTransform;

    return writer;
}

MemoryReader& operator>>(MemoryReader& reader, Level::Chunk& chunk)
{
    reader >> chunk.EntityElements;

    return reader;
}

MemoryWriter& operator<<(MemoryWriter& writer, const Level::Chunk& chunk)
{
    writer << chunk.EntityElements;

    return writer;
}
