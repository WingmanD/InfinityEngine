﻿#pragma once

#include "Object.h"
#include "PassKey.h"
#include "Name.h"
#include "Importer.h"
#include "Asset.reflection.h"

class Importer;
class AssetManager;

REFLECTED(CustomSerialization)
class Asset : public Object
{
    GENERATED()

public:
    Asset() = default;
    explicit Asset(Name name);

    virtual bool Initialize();

    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

    void SetAssetPath(const std::filesystem::path& path);
    [[nodiscard]] const std::filesystem::path& GetAssetPath() const;

    void SetAssetID(uint64 id, PassKey<AssetManager>);
    [[nodiscard]] uint64 GetAssetID() const;

    void SetName(Name name);
    [[nodiscard]] Name GetName() const;

    void SetIsLoaded(bool value, PassKey<AssetManager>);
    bool IsLoaded() const;

    bool Load();
    bool Save() const;

    void LoadDescription(MemoryReader& reader, PassKey<AssetManager>);
    void SaveDescription(MemoryWriter& writer, PassKey<AssetManager>) const;

    void SetImportPath(const std::filesystem::path& path);
    [[nodiscard]] const std::filesystem::path& GetImportPath() const;

    const Type* GetImporterType() const;

    SharedObjectPtr<Widget> CreateImportWidget() const;
    virtual SharedObjectPtr<Widget> CreateEditWidget();

    virtual DArray<SharedObjectPtr<Asset>> Import(const SharedObjectPtr<Importer>& importer) const;

    // Object
public:
    void OnPropertyChanged(Name propertyName) override;

protected:
    virtual bool LoadInternal(std::ifstream& file);
    virtual void PostLoad();
    virtual bool SaveInternal(std::ofstream& file) const;
    
    void SetIsLoaded(bool value);
    void MarkDirtyForAutosave() const;

    void SetImporterType(const Type* type);

private:
    uint64 _id = 0;

    PROPERTY(Edit, DisplayName = "Name")
    Name _name;

    PROPERTY(Visible, DisplayName = "Import Path")
    std::filesystem::path _importPath;

    PROPERTY(Visible, DisplayName = "Asset Path")
    std::filesystem::path _assetPath;

    bool _isLoaded = false;

    // todo this should be in ImportableAsset, but right now we don't have asset reparenting
    const Type* _importerType = nullptr;
};
