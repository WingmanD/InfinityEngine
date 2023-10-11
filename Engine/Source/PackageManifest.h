#pragma once

#include <filesystem>
#include <unordered_map>

class PackageManifest
{
public:
    PackageManifest() = default;
    
    static std::unique_ptr<PackageManifest> Load(const std::filesystem::path& manifestFilePath);

    static std::unique_ptr<PackageManifest> LoadOrCreate(const std::filesystem::path& manifestFilePath);

    bool Save() const;

    bool RegisterAsset(uint64_t assetID, std::streampos pos);

private:
    std::filesystem::path manifestFilePath;
    std::unordered_map<uint64_t, std::streampos> _assetMap;
};
