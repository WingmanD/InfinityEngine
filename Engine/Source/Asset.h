#pragma once

#include "Object.h"
#include "ISerializeable.h"
#include "MemoryReader.h"
#include "MemoryWriter.h"
#include "PassKey.h"
#include <string>
#include "Asset.reflection.h"

class AssetManager;

REFLECTED()
class Asset : public Object, public ISerializeable
{
    GENERATED()

public:
    Asset() = default;
    explicit Asset(std::wstring name);

    virtual bool Initialize();

    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

    void SetAssetPath(const std::filesystem::path& path);
    [[nodiscard]] const std::filesystem::path& GetAssetPath() const;

    void SetAssetID(uint64 id, PassKey<AssetManager>);
    [[nodiscard]] uint64 GetAssetID() const;

    void SetName(const std::wstring& name);
    [[nodiscard]] const std::wstring& GetName() const;

    void SetIsLoaded(bool value, PassKey<AssetManager>);
    bool IsLoaded() const;

    bool Load();
    bool Save() const;

    void LoadDescription(MemoryReader& reader, PassKey<AssetManager>);
    void SaveDescription(MemoryWriter& writer, PassKey<AssetManager>) const;

    void SetImportPath(const std::filesystem::path& path);
    [[nodiscard]] const std::filesystem::path& GetImportPath() const;

protected:
    void SetIsLoaded(bool value);
    void MarkDirtyForAutosave() const;

private:
    uint64 _id = 0;

    PROPERTY(Edit, DisplayName = "Name")
    std::wstring _name;

    PROPERTY(DisplayName = "Import Path")
    std::filesystem::path _importPath;

    PROPERTY(DisplayName = "Asset Path")
    std::filesystem::path _assetPath;

    bool _isLoaded = false;
};
