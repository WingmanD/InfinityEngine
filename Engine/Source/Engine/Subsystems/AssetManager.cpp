#include <iostream>
#include "AssetManager.h"

AssetManager::~AssetManager()
{
    const bool success = _manifest->Save();
    if (!success)
    {
        LOG(L"Failed to save manifest!");
    }

    _assetCache.close();
}

bool AssetManager::RegisterAsset(const std::shared_ptr<Asset>& asset)
{
    if (asset == nullptr)
    {
        return false;
    }
    
    _assetMap[asset->GetAssetID()] = asset;

    return true;
}

bool AssetManager::UnregisterAsset(const std::shared_ptr<Asset>& asset)
{
    if (asset == nullptr)
    {
        return false;
    }

    return _assetMap.erase(asset->GetAssetID()) > 0;
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

bool AssetManager::Initialize()
{
    _projectRootPath = FindProjectRootPath();
    LOG(L"Project root path: {}", _projectRootPath.wstring());

    _manifest = PackageManifest::LoadOrCreate(_projectRootPath / "Engine" / "Content" / "AssetCache.manifest");
    if (_manifest == nullptr)
    {
        LOG(L"Failed to load or create manifest!");
        return false;
    }

    _assetCache.open(_projectRootPath / "Engine" / "Content" / "AssetCache.asset",
                     std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
    if (!_assetCache.is_open())
    {
        LOG(L"Failed to open asset cache!");
        return false;
    }

    return true;
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
