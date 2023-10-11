#include "PackageManifest.h"
#include "Core.h"
#include "MemoryReader.h"
#include "MemoryWriter.h"
#include <fstream>

std::unique_ptr<PackageManifest> PackageManifest::Load(const std::filesystem::path& manifestFilePath)
{
    std::unique_ptr<PackageManifest> manifest = std::make_unique<PackageManifest>();
    manifest->manifestFilePath = manifestFilePath;

    std::ifstream file(manifestFilePath, std::ios::binary);
    if (!file.is_open())
    {
        return nullptr;
    }

    MemoryReader reader;
    if (!reader.ReadFromFile(file))
    {
        return nullptr;
    }

    uint64 num = 0;
    reader >> num;

    for (uint64 i = 0; i < num; ++i)
    {
        uint64 assetID;
        reader >> assetID;

        int64 pos;
        reader >> pos;

        manifest->_assetMap.emplace(assetID, pos);
    }

    return manifest;
}

std::unique_ptr<PackageManifest> PackageManifest::LoadOrCreate(const std::filesystem::path& manifestFilePath)
{
    if (!exists(manifestFilePath))
    {
        create_directories(manifestFilePath.parent_path());
        
        std::ofstream file(manifestFilePath, std::ios::binary);
        if (!file.is_open())
        {
            LOG(L"Could not open manifest file {} for writing!", manifestFilePath.wstring());
            return nullptr;
        }
        
        MemoryWriter writer;
        writer << 0ull;
        writer.WriteToFile(file);
        
        file.close();
    }

    return Load(manifestFilePath);
}

bool PackageManifest::Save() const
{
    std::ofstream file(manifestFilePath, std::ios::binary);
    if (!file.is_open())
    {
        LOG(L"Could not open manifest file {} for writing!", manifestFilePath.wstring());
        return false;
    }

    MemoryWriter writer;
    writer << _assetMap.size();
    for (auto& [id, pos] : _assetMap)
    {
        int64 intPos = pos;
        writer << id << intPos;
    }

    return writer.WriteToFile(file);
}

bool PackageManifest::RegisterAsset(uint64_t assetID, std::streampos pos)
{
    _assetMap.emplace(assetID, pos);
    return true;
}
