#include "AssetManager.h"
#include "Engine/Engine.h"
#include "Util.h"
#include "Rendering/StaticMesh.h"
#include "Rendering/Widgets/UIStatics.h"
#include <shobjidl.h>
#include <Windows.h>

AssetManager& AssetManager::Get()
{
    return Engine::Get().GetAssetManager();
}

AssetManager::~AssetManager()
{
    if (_manifest != nullptr)
    {
        const bool success = _manifest->Save();
        if (!success)
        {
            LOG(L"Failed to save manifest!");
        }
    }

    _assetCache.close();
}

std::shared_ptr<Asset> AssetManager::NewAsset(const Type* type, const std::wstring& name)
{
    if (type == nullptr)
    {
        return nullptr;
    }

    std::shared_ptr<Asset> asset = type->NewObject<Asset>();
    asset->SetName(name);
    asset->SetAssetID(_idGenerator.GenerateID(), {});

    if (!RegisterAsset(asset))
    {
        LOG(L"Failed to register asset {}!", name);
        return nullptr;
    }

    const std::filesystem::path assetPath = _assetCacheDirectory / (name + L".asset");
    std::ofstream file(assetPath, std::ios::binary);
    if (!file.is_open())
    {
        LOG(L"Failed to create file for asset {}!", name);
        return nullptr;
    }
    file.close();
    asset->SetAssetPath(assetPath);
    asset->SetIsLoaded(true, {});

    MarkDirtyForAutosave(asset);

    return asset;
}

void AssetManager::DeleteAsset(const std::shared_ptr<Asset>& asset)
{
    if (asset == nullptr)
    {
        return;
    }

    if (!std::filesystem::remove(asset->GetAssetPath()))
    {
        LOG(L"Failed to delete asset file!");
    }

    UnregisterAsset(asset);
    if (asset.use_count() != 1)
    {
        LOG(L"Asset {} has been deleted, but there are still references to it in memory.", asset->GetName());
    }
}

bool AssetManager::ForEachAssetOfType(Type* type, const std::function<bool(const std::shared_ptr<Asset>&)>& callback,
                                      bool recursive /*= false*/) const
{
    if (type == nullptr)
    {
        return false;
    }

    const auto it = _assetTypeMap.find(type);
    if (it != _assetTypeMap.end())
    {
        for (const uint64 id : it->second)
        {
            if (const std::shared_ptr<Asset> asset = FindAsset(id))
            {
                if (!callback(asset))
                {
                    return false;
                }
            }
        }
    }
    
    if (recursive)
    {
        for (Type* subtype : type->GetSubtypes())
        {
            return ForEachAssetOfType(subtype, callback, true);
        }
    }

    return true;
}

bool AssetManager::RegisterAsset(const std::shared_ptr<Asset>& asset)
{
    if (asset == nullptr)
    {
        return false;
    }

    if (asset->GetAssetID() == 0)
    {
        LOG(L"Failed to register asset {} - asset id is 0!", asset->GetName());
        return false;
    }

    if (_assetMap.contains(asset->GetAssetID()))
    {
        LOG(L"Failed to register asset {} - asset id {} is already registered!", asset->GetName(), asset->GetAssetID());
        return false;
    }

    if (_assetNameMap.contains(asset->GetName()))
    {
        LOG(L"Failed to register asset {} - asset name {} is already registered!", asset->GetName(),
            asset->GetAssetID());
        return false;
    }

    //_manifest->RegisterAsset();

    _assetMap[asset->GetAssetID()] = asset;
    _assetNameMap[asset->GetName()] = asset->GetAssetID();
    _assetTypeMap[asset->GetType()].push_back(asset->GetAssetID());

    OnAssetMapChanged();

    return true;
}

bool AssetManager::UnregisterAsset(const std::shared_ptr<Asset>& asset)
{
    if (asset == nullptr)
    {
        return false;
    }

    _assetNameMap.erase(asset->GetName());

    _assetTypeMap[asset->GetType()].erase(std::ranges::find(_assetTypeMap[asset->GetType()],
                                                            asset->GetAssetID()));

    std::erase_if(_assetTypeMap[asset->GetType()],
                  [asset](uint64 id)
                  {
                      return asset->GetAssetID() == id;
                  });

    if (_assetMap.erase(asset->GetAssetID()) > 0)
    {
        OnAssetMapChanged();

        return true;
    }

    return false;
}

void AssetManager::MarkDirtyForAutosave(const std::shared_ptr<const Asset>& asset)
{
    if (asset == nullptr)
    {
        return;
    }

    _assetIDsToAutosave.insert(asset->GetAssetID());
}

void AssetManager::RediscoverAssets()
{
    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(_assetCacheDirectory))
    {
        if (!dirEntry.path().string().ends_with(".asset"))
        {
            continue;
        }

        std::ifstream file(dirEntry.path(), std::ios::binary);
        if (!file.is_open())
        {
            LOG(L"Failed to open asset file {}!", dirEntry.path().wstring());
            continue;
        }

        MemoryReader reader;
        if (!reader.ReadFromFile(file))
        {
            LOG(L"Failed to read asset file {}!", dirEntry.path().wstring());
            continue;
        }

        uint64 typeID;
        reader >> typeID;

        const Type* type = TypeRegistry::Get().FindTypeForID(typeID);
        if (type == nullptr)
        {
            LOG(L"Failed to find type for asset file {}!", dirEntry.path().wstring());
            continue;
        }

        uint64 assetID;
        reader >> assetID;

        if (!FindAsset(assetID))
        {
            std::shared_ptr<Asset> existingAsset = type->NewObject<Asset>();

            reader.ResetOffset();

            if (!existingAsset->Deserialize(reader))
            {
                LOG(L"Failed to deserialize asset from file {}!", dirEntry.path().wstring());
                continue;
            }
            existingAsset->SetAssetPath(dirEntry);

            if (!RegisterAsset(existingAsset))
            {
                LOG(L"Failed to register asset from file {}!", dirEntry.path().wstring());
                continue;
            }
        }
    }
}

void AssetManager::ImportFromDialog(const Type* type)
{
    if (type == nullptr)
    {
        return;
    }

    std::filesystem::path path;

    HRESULT hr = CoInitializeEx(nullptr,
                                COINIT_APARTMENTTHREADED |
                                COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog* pFileOpen;

        hr = CoCreateInstance(CLSID_FileOpenDialog,
                              nullptr,
                              CLSCTX_ALL,
                              IID_IFileOpenDialog,
                              reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr))
        {
            hr = pFileOpen->Show(nullptr);

            if (SUCCEEDED(hr))
            {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                    if (SUCCEEDED(hr))
                    {
                        const std::wstring ws(pszFilePath);
                        path = ws;

                        CoTaskMemFree(pszFilePath);
                    }

                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();

        const Asset* asset = type->GetCDO<Asset>();
        if (asset == nullptr)
        {
            LOG(L"Failed to import asset - type {} is not an asset!", Util::ToWString(type->GetName()));
            return;
        }

        // todo link BatchImport and Import to ImportFromDialog somehow - probably using virtual methods that return static function result

        // todo temporary
        if (type->IsA(StaticMesh::StaticType()))
        {
            std::vector<std::shared_ptr<StaticMesh>> meshes = StaticMesh::BatchImport(path);
        }
    }
}

std::shared_ptr<Asset> AssetManager::FindAssetByName(const std::wstring& name) const
{
    const auto it = _assetNameMap.find(name);
    if (it != _assetNameMap.end())
    {
        return FindAsset(it->second);
    }

    return nullptr;
}

std::shared_ptr<Asset> AssetManager::FindAsset(uint64 id) const
{
    const auto it = _assetMap.find(id);

    if (it != _assetMap.end())
    {
        return it->second;
    }

    return nullptr;
}

const std::filesystem::path& AssetManager::GetProjectRootPath() const
{
    return _projectRootPath;
}

void AssetManager::LoadAlwaysLoadedAssets()
{
    // todo make a list of always loaded assets and load them here instead of this
    UIStatics::GetUIQuadMesh()->Load();
}

bool AssetManager::Initialize()
{
    _projectRootPath = FindProjectRootPath();
    _assetCacheDirectory = _projectRootPath / "Engine" / "Content";

    if (!Load())
    {
        return false;
    }

    _manifest = PackageManifest::LoadOrCreate(_assetCacheDirectory / "AssetCache.manifest");
    if (_manifest == nullptr)
    {
        LOG(L"Failed to load or create manifest!");
        return false;
    }

    _assetCache.open(_assetCacheDirectory / "AssetCache.pak",
                     std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
    if (!_assetCache.is_open())
    {
        LOG(L"Failed to open asset cache!");
        return false;
    }

    return true;
}

void AssetManager::Shutdown()
{
    _assetCache.close();

    Save();

    // if (!_manifest->Save())
    // {
    //     LOG(L"Failed to save AssetCache manifest!");
    // }
    //
    // std::ofstream assetCache(_assetCacheDirectory / "AssetCache.asset", std::ios::binary);
    // if (!assetCache.is_open())
    // {
    //     LOG(L"Failed to save AssetCache - failed to open AssetCache!");
    //     return;
    // }
    //
    // MemoryWriter writer;
    // for (auto [id, asset] : _assetMap)
    // {
    //     writer << id;
    //     asset->Serialize(writer);
    // }
    //
    // writer.WriteToFile(assetCache);
}

std::filesystem::path AssetManager::FindProjectRootPath() const
{
    wchar_t executablePath[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, executablePath, MAX_PATH))
    {
        return "";
    }

#if DEBUG
    return std::filesystem::path(executablePath).parent_path().parent_path().parent_path().parent_path();
#else
    // todo packaged game path
    return executablePath;
#endif
}

void AssetManager::OnAssetMapChanged()
{
    Save();
}

bool AssetManager::Load()
{
    const std::filesystem::path assetMapPath = _assetCacheDirectory / "EditorAssetMap.pak";
    std::ifstream editorAssetMap(assetMapPath, std::ios::binary);
    if (!editorAssetMap.is_open())
    {
        LOG(L"Failed to open editor asset map, attempting to create a new one...");

        std::ofstream file(assetMapPath, std::ios::binary);
        if (!file.is_open())
        {
            LOG(L"Failed to create editor asset map!");
            return false;
        }

        return true;
    }

    MemoryReader reader;
    reader.ReadFromFile(editorAssetMap);

    if (reader.GetNumRemainingBytes() < sizeof(size_t))
    {
        return true;
    }

    _idGenerator.Deserialize(reader);

    uint64 assetCount;
    reader >> assetCount;

    for (size_t i = 0; i < assetCount; ++i)
    {
        uint64 assetTypeID;
        reader >> assetTypeID;

        uint64 assetID;
        reader >> assetID;

        std::filesystem::path assetPath;
        reader >> assetPath;

        Type* type = TypeRegistry::Get().FindTypeForID(assetTypeID);
        if (type == nullptr)
        {
            LOG(L"Loading asset description failed - unknown type {}...", assetPath.wstring());
            continue;
        }

        std::shared_ptr<Asset> asset = type->NewObject<Asset>();
        if (asset == nullptr)
        {
            LOG(L"Loading asset description failed - failed to create asset {}...", assetPath.wstring());
            continue;
        }

        asset->SetAssetID(assetID, {});
        asset->SetAssetPath(assetPath);

        // todo this is a problem - we should read description first, then create asset - when we delete asset type, we can't deserialize it, which breaks all other asset descriptions
        LOG(L"Loading asset description {}...", assetPath.wstring());
        asset->LoadDescription(reader, {});

        if (exists(assetPath))
        {
            _assetMap[asset->GetAssetID()] = asset;
            _assetNameMap[asset->GetName()] = asset->GetAssetID();
            _assetTypeMap[type].push_back(asset->GetAssetID());
        }
        else
        {
            LOG(L"Asset {} does not exist!", assetPath.wstring());
        }
    }

    RediscoverAssets();

    return true;
}

bool AssetManager::Save()
{
    AutosaveAssets();

    std::ofstream editorAssetMap(_assetCacheDirectory / "EditorAssetMap.pak", std::ios::binary);
    if (!editorAssetMap.is_open())
    {
        LOG(L"Failed to open editor asset map!");
        return false;
    }

    MemoryWriter writer;

    _idGenerator.Serialize(writer);

    const uint64 assetCount = _assetMap.size();
    writer << assetCount;
    for (const std::shared_ptr<Asset> asset : _assetMap | std::views::values)
    {
        writer << asset->GetType()->GetID();
        writer << asset->GetAssetID();
        writer << asset->GetAssetPath();

        LOG(L"Saving asset description {}...", asset->GetName());
        asset->SaveDescription(writer, {});
    }
    writer.WriteToFile(editorAssetMap);

    return true;
}

void AssetManager::AutosaveAssets() const
{
    for (const uint64 assetID : _assetIDsToAutosave)
    {
        if (const std::shared_ptr<const Asset> asset = FindAsset(assetID))
        {
            LOG(L"Saving asset {}...", asset->GetName());
            if (!asset->Save())
            {
                LOG(L"Failed to save asset {}!", asset->GetName());
            }
        }
    }
}
