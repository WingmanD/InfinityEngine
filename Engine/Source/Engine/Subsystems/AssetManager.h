#pragma once

#include "Engine/Subsystems/EngineSubsystem.h"
#include "Asset.h"
#include "PackageManifest.h"
#include "IDGenerator.h"
#include <fstream>
#include <set>

class AssetManager : public EngineSubsystem
{
public:
    Delegate<std::shared_ptr<Asset>> OnAssetCreated;
    Delegate<std::shared_ptr<Asset>> OnAssetDeleted;
    
public:
    static AssetManager& Get();

    ~AssetManager() override;

    std::shared_ptr<Asset> NewAsset(const Type* type, const std::wstring& name);

    template <typename T> requires IsA<T, Asset>
    std::shared_ptr<T> NewAsset(const std::wstring& name) requires IsA<T, Asset>
    {
        return std::dynamic_pointer_cast<T>(NewAsset(T::StaticType(), name));
    }

    void DeleteAsset(const std::shared_ptr<Asset>& asset);

    template <typename T, typename... Args>
    std::shared_ptr<T> Import(const std::filesystem::path& path, Args&&... args) requires IsA<T, Asset>
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

    std::shared_ptr<Asset> FindAssetByName(const std::wstring& name) const;

    template <typename T>
    std::shared_ptr<T> FindAssetByName(const std::wstring& name) const requires IsA<T, Asset>
    {
        return std::dynamic_pointer_cast<T>(FindAssetByName(name));
    }

    std::shared_ptr<Asset> FindAsset(uint64 id) const;

    template <typename T>
    std::shared_ptr<T> FindAsset(uint64 id) const requires IsA<T, Asset>
    {
        return std::dynamic_pointer_cast<T>(FindAsset(id));
    }

    bool ForEachAssetOfType(Type* type, const std::function<bool(const std::shared_ptr<Asset>&)>& callback, bool recursive = false) const;
    void ForEachAsset(const std::function<bool(std::shared_ptr<Asset>&)>& callback);

    bool RegisterAsset(const std::shared_ptr<Asset>& asset);
    bool UnregisterAsset(const std::shared_ptr<Asset>& asset);

    void MarkDirtyForAutosave(const std::shared_ptr<const Asset>& asset);

    void RediscoverAssets();

    const std::filesystem::path& GetProjectRootPath() const;

    void LoadAlwaysLoadedAssets();

protected:
    virtual bool Initialize() override;
    virtual void Shutdown() override;

private:
    IDGenerator _idGenerator;
    
    std::unique_ptr<PackageManifest> _manifest;
    std::fstream _assetCache;
    
    std::unordered_map<uint64, std::shared_ptr<Asset>> _assetMap;
    std::unordered_map<std::wstring, uint64> _assetNameMap;
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
