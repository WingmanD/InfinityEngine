#pragma once

#include "Engine/Subsystems/EngineSubsystem.h"
#include "Singleton.h"
#include "Asset.h"
#include <fstream>

#include "PackageManifest.h"
#include "PassKey.h"

template <typename Base, typename Derived>
concept IsA = std::is_base_of_v<Base, Base>;

class AssetManager : public EngineSubsystem
{
public:
    ~AssetManager() override;

    template <typename T>
    std::shared_ptr<T> NewAsset(const std::string& name) requires IsA<Asset, T>
    {
        auto asset = std::make_shared<T>(name);
        RegisterAsset(asset);
        return asset;
    }

    template <typename T, typename... Params>
    std::shared_ptr<T> Import(Params... params) requires IsA<Asset, T>
    {
        auto asset = T::Import(params...);
        RegisterAsset(asset);
        return asset;
    }

    std::shared_ptr<Asset> FindAsset(uint64 id) const;

    template <typename T>
    std::shared_ptr<T> FindAsset(uint64 id) const requires IsA<Asset, T>
    {
        return std::dynamic_pointer_cast<T>(FindAsset(id));
    }

    const std::filesystem::path& GetProjectRootPath() const;

public:
    virtual bool Initialize() override;

private:
    std::unique_ptr<PackageManifest> _manifest;
    std::fstream _assetCache;

    std::unordered_map<uint64, std::shared_ptr<Asset>> _assetMap;

    std::filesystem::path _projectRootPath;

private:
    std::filesystem::path FindProjectRootPath() const;

    bool RegisterAsset(const std::shared_ptr<Asset>& asset);
    bool UnregisterAsset(const std::shared_ptr<Asset>& asset);
};
