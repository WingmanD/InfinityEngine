#pragma once

#include "Object.h"
#include "MemoryReader.h"
#include "MemoryWriter.h"
#include <string>
#include "Asset.reflection.h"
#include "PassKey.h"

class AssetManager;

REFLECTED()
class Asset : public Object
{
    ASSET_GENERATED()

public:
    Asset() = default;
    // todo fix this mess
    explicit Asset(std::string name);
    explicit Asset(std::string name, std::filesystem::path importPath);

    virtual bool Serialize(MemoryWriter& writer);

    // todo deserialize must be static and return Asset*
    virtual bool Deserialize(MemoryReader& reader);

    [[nodiscard]] uint64_t GetAssetID() const;

    [[nodiscard]] const std::string& GetName() const;
    
private:
    uint64_t _id = 0;

    std::string _name;

    PROPERTY(VisibleInEditor, DisplayName = "Import Path")
    std::filesystem::path _importPath;
};
