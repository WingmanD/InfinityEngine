#pragma once

#include "Engine/Subsystems/EngineSubsystem.h"
#include "Asset.h"
#include "Name.h"
#include "PackageManifest.h"
#include "IDGenerator.h"
#include <fstream>
#include <set>

class AssetManager : public EngineSubsystem
{
public:
    MulticastDelegate<SharedObjectPtr<Asset>> OnAssetCreated;
    MulticastDelegate<SharedObjectPtr<Asset>> OnAssetDeleted;
    
public:
    static AssetManager& Get();

    virtual ~AssetManager() override;

    SharedObjectPtr<Asset> NewAsset(const Type& type, Name name);

    template <typename T> requires IsA<T, Asset>
    SharedObjectPtr<T> NewAsset(Name name) requires IsA<T, Asset>
    {
        return std::dynamic_pointer_cast<T>(NewAsset(*T::StaticType(), name));
    }

    void DeleteAsset(const SharedObjectPtr<Asset>& asset);

    template <typename T, typename... Args>
    SharedObjectPtr<T> Import(const std::filesystem::path& path, Args&&... args) requires IsA<T, Asset>
    {
        std::filesystem::path actualPath = path;
        if (path.is_relative())
        {
            actualPath = GetProjectRootPath() / path;
        }

        auto asset = T::Import(*this, actualPath, std::forward<Args>(args)...);
        if (asset == nullptr)
        {
            return nullptr;
        }
        
        asset->SetImportPath(actualPath);
        asset->SetIsLoaded(true, {});
        
        return asset;
    }

    SharedObjectPtr<Asset> FindAssetByName(Name name) const;

    template <typename T>
    SharedObjectPtr<T> FindAssetByName(Name name) const requires IsA<T, Asset>
    {
        return std::dynamic_pointer_cast<T>(FindAssetByName(name));
    }

    SharedObjectPtr<Asset> FindOrCreateAssetByName(const Type& type, Name name);
    
    template <typename T>
    SharedObjectPtr<T> FindOrCreateAssetByName(Name name) requires IsA<T, Asset>
    {
        return std::dynamic_pointer_cast<T>(FindOrCreateAssetByName(*T::StaticType(), name));
    }

    SharedObjectPtr<Asset> FindAsset(uint64 id) const;

    template <typename T>
    SharedObjectPtr<T> FindAsset(uint64 id) const requires IsA<T, Asset>
    {
        return std::dynamic_pointer_cast<T>(FindAsset(id));
    }

    bool ForEachAssetOfType(Type* type, const std::function<bool(const SharedObjectPtr<Asset>&)>& callback, bool recursive = false) const;
    void ForEachAsset(const std::function<bool(SharedObjectPtr<Asset>&)>& callback);

    bool RegisterAsset(const SharedObjectPtr<Asset>& asset);
    bool UnregisterAsset(const SharedObjectPtr<Asset>& asset);

    void MarkDirtyForAutosave(const SharedObjectPtr<const Asset>& asset);

    void RediscoverAssets();

    const std::filesystem::path& GetProjectRootPath() const;

    void LoadAlwaysLoadedAssets();

protected:
    virtual bool Initialize() override;
    virtual void Shutdown() override;

private:
    IDGenerator<uint64> _idGenerator;
    
    std::unique_ptr<PackageManifest> _manifest;
    std::fstream _assetCache;
    
    std::unordered_map<uint64, SharedObjectPtr<Asset>> _assetMap;
    std::unordered_map<Name, uint64> _assetNameMap;
    std::unordered_map<Type*, std::vector<uint64>> _assetTypeMap;

    std::filesystem::path _projectRootPath;
    std::filesystem::path _assetCacheDirectory;

    std::set<uint64> _assetIDsToAutosave;

private:
    std::filesystem::path FindProjectRootPath() const;

    void OnAssetMapChanged();
    bool Load();
    bool Save();

    void AutosaveAssets() const;
};
