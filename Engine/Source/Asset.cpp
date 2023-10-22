#include "Asset.h"

#include <fstream>

#include "Engine/Subsystems/AssetManager.h"

Asset::Asset(std::wstring name) : _name(std::move(name))
{
    // todo ask AssetManager for a new ID
    _id = std::hash<std::wstring>{}(_name);
}

bool Asset::Initialize()
{
    return true;
}

bool Asset::Serialize(MemoryWriter& writer) const
{
    writer << GetType()->GetID();
    writer << _id;
    writer << _name;
    writer << _importPath;

    return true;
}

bool Asset::Deserialize(MemoryReader& reader)
{
    uint64 typeID;
    reader >> typeID;
    reader >> _id;
    reader >> _name;
    reader >> _importPath;

    return true;
}

void Asset::SetAssetPath(const std::filesystem::path& path)
{
    _assetPath = path;

    MarkDirtyForAutosave();
}

const std::filesystem::path& Asset::GetAssetPath() const
{
    return _assetPath;
}

void Asset::SetAssetID(uint64 id, PassKey<AssetManager>)
{
    _id = id;
}

uint64 Asset::GetAssetID() const
{
    return _id;
}

void Asset::SetName(const std::wstring& name)
{
    _name = name;

    // todo notify asset manager
    MarkDirtyForAutosave();
}

const std::wstring& Asset::GetName() const
{
    return _name;
}

void Asset::SetIsLoaded(bool value, PassKey<AssetManager>)
{
    SetIsLoaded(value);
}

bool Asset::IsLoaded() const
{
    return _isLoaded;
}

void Asset::Load()
{
    // todo this must be refactored for packaged builds
    if (_isLoaded)
    {
        return;
    }

    LOG(L"Loading asset {}...", _name);
    
    std::ifstream file(_assetPath, std::ios::binary);
    if (!file.is_open())
    {
        LOG(L"Failed to open file {} for reading!", _assetPath.wstring());
        return;
    }

    MemoryReader reader;
    if (!reader.ReadFromFile(file))
    {
        LOG(L"Failed to read asset {} from file {}!", _name, _assetPath.wstring());
        return;
    }

    if (!Deserialize(reader))
    {
        LOG(L"Failed to deserialize asset {} from file {}!", _name, _assetPath.wstring());
        return;
    }

    GetType()->ForEachPropertyWithTag("Load", [this](PropertyBase* property)
    {
        const std::shared_ptr<Asset> valueRef = static_cast<Property<Asset, std::shared_ptr<Asset>>*>(property)->GetRef(this);
        if (valueRef == nullptr)
        {
            return;
        }
        
        valueRef->Load();
    });

    _isLoaded = true;

    if (!Initialize())
    {
        LOG(L"Failed to initialize asset {}!", _name);
        DEBUG_BREAK();
    }
}

void Asset::Save() const
{
    if (!_isLoaded)
    {
        return;
    }
    
    if (_assetPath.empty())
    {
        LOG(L"Asset {} has no asset path!", _name);
        return;
    }

    std::ofstream file(_assetPath, std::ios::binary);
    if (!file.is_open())
    {
        LOG(L"Failed to open file {} for writing!", _assetPath.wstring());
        return;
    }

    MemoryWriter writer;
    Serialize(writer);
    if (!writer.WriteToFile(file))
    {
        LOG(L"Failed to write asset {} to file {}!", _name, _assetPath.wstring());
    }
}

void Asset::LoadDescription(MemoryReader& reader, PassKey<AssetManager>)
{
    reader >> _id;
    reader >> _name;
}

void Asset::SaveDescription(MemoryWriter& writer, PassKey<AssetManager>) const
{
    writer << _id;
    writer << _name;
}

void Asset::SetImportPath(const std::filesystem::path& path)
{
    _importPath = path;
    MarkDirtyForAutosave();
}

const std::filesystem::path& Asset::GetImportPath() const
{
    return _importPath;
}

void Asset::SetIsLoaded(bool value)
{
    _isLoaded = value;
}

void Asset::MarkDirtyForAutosave() const
{
    // todo put this under if EDITOR and not during play
    AssetManager::Get().MarkDirtyForAutosave(std::dynamic_pointer_cast<const Asset>(shared_from_this()));
}
