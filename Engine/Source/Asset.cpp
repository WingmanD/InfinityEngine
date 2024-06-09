#include "Asset.h"
#include "AssetPtrBase.h"
#include "Importer.h"
#include "Engine/Subsystems/AssetManager.h"
#include "Rendering/Widgets/Button.h"
#include "Rendering/Widgets/FlowBox.h"
#include "Rendering/Widgets/TextBox.h"
#include <fstream>

Asset::Asset(Name name) : _name(name)
{
    // todo ask AssetManager for a new ID
    _id = _name.GetID();
}

bool Asset::Initialize()
{
    return true;
}

bool Asset::Serialize(MemoryWriter& writer) const
{
    writer << GetType()->GetID();
    writer << _id;
    writer << _name;
    writer << _importPath;

    return true;
}

bool Asset::Deserialize(MemoryReader& reader)
{
    uint64 typeID;
    reader >> typeID;
    reader >> _id;
    reader >> _name;
    reader >> _importPath;

    return true;
}

void Asset::SetAssetPath(const std::filesystem::path& path)
{
    _assetPath = path;

    MarkDirtyForAutosave();
}

const std::filesystem::path& Asset::GetAssetPath() const
{
    return _assetPath;
}

void Asset::SetAssetID(uint64 id, PassKey<AssetManager>)
{
    _id = id;
}

uint64 Asset::GetAssetID() const
{
    return _id;
}

void Asset::SetName(Name name)
{
    _name = name;

    // todo notify asset manager
    MarkDirtyForAutosave();
}

Name Asset::GetName() const
{
    return _name;
}

void Asset::SetIsLoaded(bool value, PassKey<AssetManager>)
{
    SetIsLoaded(value);
}

bool Asset::IsLoaded() const
{
    return _isLoaded;
}

bool Asset::Load()
{
    // todo this must be refactored for packaged builds
    if (_isLoaded)
    {
        return true;
    }

    LOG(L"Loading asset {}...", _name.ToString());

    std::ifstream file(_assetPath, std::ios::binary);
    if (!file.is_open())
    {
        LOG(L"Failed to open file {} for reading (asset {})!", _assetPath.wstring(), _name.ToString());
        return false;
    }

    if (!LoadInternal(file))
    {
        LOG(L"Failed to load asset {}!", _name.ToString());
        return false;
    }

    GetType()->ForEachPropertyWithTag("Load", [this](PropertyBase* property)
    {
        // todo this should be dynamic cast
        const std::shared_ptr<Asset> valueRef = static_cast<Property<Asset, AssetPtrBase>*>(property)->GetRef(this);
        if (valueRef == nullptr)
        {
            return true;
        }

        valueRef->Load();

        return true;
    });

    _isLoaded = true;

    if (!Initialize())
    {
        LOG(L"Failed to initialize asset {}!", _name.ToString());
        DEBUG_BREAK();
        return false;
    }

    PostLoad();

    return true;
}

bool Asset::Save() const
{
    if (!_isLoaded)
    {
        return false;
    }

    if (_assetPath.empty())
    {
        LOG(L"Asset {} has no asset path!", _name.ToString());
        return false;
    }

    std::ofstream file(_assetPath, std::ios::binary);
    if (!file.is_open())
    {
        LOG(L"Failed to open file {} for writing!", _assetPath.wstring());
        return false;
    }

    if (!SaveInternal(file))
    {
        LOG(L"Failed to save asset {}!", _name.ToString());
        return false;
    }

    return true;
}

void Asset::LoadDescription(MemoryReader& reader, PassKey<AssetManager>)
{
    reader >> _id;
    reader >> _name;
}

void Asset::SaveDescription(MemoryWriter& writer, PassKey<AssetManager>) const
{
    writer << _id;
    writer << _name;
}

void Asset::SetImportPath(const std::filesystem::path& path)
{
    _importPath = path;
    MarkDirtyForAutosave();
}

const std::filesystem::path& Asset::GetImportPath() const
{
    return _importPath;
}

const Type* Asset::GetImporterType() const
{
    return _importerType;
}

std::shared_ptr<Widget> Asset::CreateImportWidget() const
{
    if (_importerType == nullptr)
    {
        DEBUG_BREAK();
        return nullptr;
    }

    const std::shared_ptr<FlowBox> verticalBox = std::make_shared<FlowBox>();
    if (!verticalBox->Initialize())
    {
        return nullptr;
    }

    verticalBox->SetDirection(EFlowBoxDirection::Vertical);

    {
        const std::shared_ptr<FlowBox> horizontalBox = verticalBox->AddChild<FlowBox>();
        if (horizontalBox == nullptr)
        {
            return nullptr;
        }
        horizontalBox->SetDirection(EFlowBoxDirection::Horizontal);
        horizontalBox->SetFillMode(EWidgetFillMode::FillX);

        const std::shared_ptr<TextBox> title = horizontalBox->AddChild<TextBox>();
        if (title == nullptr)
        {
            return nullptr;
        }

        title->SetText(std::format(L"Import {}", Util::ToWString(GetType()->GetName())));
        title->SetFillMode(EWidgetFillMode::FillX);
        title->SetPadding({0.0f, 50.0f, 0.0f, 0.0f});

        const std::shared_ptr<Button> closeButton = horizontalBox->AddChild<Button>();
        if (closeButton == nullptr)
        {
            return nullptr;
        }
        closeButton->SetText(L"x");
        closeButton->SetPadding({5.0f, 5.0f, 0.0f, 0.0f});
        closeButton->SetFillMode(EWidgetFillMode::FillY);
        std::ignore = closeButton->OnReleased.Add([verticalBox]()
        {
            verticalBox->DestroyWidget();
        });
    }

    const std::shared_ptr<Importer> importer = _importerType->NewObject<Importer>();
    const std::shared_ptr<Widget> widget = _importerType->CreatePropertiesWidget(importer);
    verticalBox->AddChild(widget);

    const std::shared_ptr<Button> importButton = verticalBox->AddChild<Button>();
    if (importButton == nullptr)
    {
        return nullptr;
    }

    importButton->SetText(L"Import");
    importButton->SetFillMode(EWidgetFillMode::FillX);

    const Type* assetType = GetType();
    std::ignore = importButton->OnReleased.Add([assetType, importer, verticalBox]()
    {
        const DArray<std::shared_ptr<Asset>> importedAssets = assetType->GetCDO<Asset>()->Import(importer);
        // todo notification
        LOG(L"Imported assets: ");
        for (const std::shared_ptr<Asset>& asset : importedAssets)
        {
            LOG(L"{}", asset->GetName().ToString());
        }

        verticalBox->DestroyWidget();
    });

    return verticalBox;
}

std::shared_ptr<Widget> Asset::CreateEditWidget()
{
    return GetType()->CreatePropertiesWidget(SharedFromThis());
}

DArray<std::shared_ptr<Asset>> Asset::Import(const std::shared_ptr<Importer>& importer) const
{
    return {};
}

void Asset::OnPropertyChanged(Name propertyName)
{
    MarkDirtyForAutosave();
}

bool Asset::LoadInternal(std::ifstream& file)
{
    MemoryReader reader;
    if (!reader.ReadFromFile(file))
    {
        LOG(L"Failed to read asset {} from file {}!", _name.ToString(), _assetPath.wstring());
        return false;
    }

    if (!Deserialize(reader))
    {
        LOG(L"Failed to deserialize asset {} from file {}!", _name.ToString(), _assetPath.wstring());
        return false;
    }
    
    return true;
}

void Asset::PostLoad()
{
}

bool Asset::SaveInternal(std::ofstream& file) const
{
    MemoryWriter writer;
    Serialize(writer);
    if (!writer.WriteToFile(file))
    {
        LOG(L"Failed to write asset {} to file {}!", _name.ToString(), _assetPath.wstring());
        return false;
    }

    return true;
}

void Asset::SetIsLoaded(bool value)
{
    _isLoaded = value;
}

void Asset::MarkDirtyForAutosave() const
{
    // todo put this under if EDITOR and not during play
    AssetManager::Get().MarkDirtyForAutosave(std::dynamic_pointer_cast<const Asset>(shared_from_this()));
}

void Asset::SetImporterType(const Type* type)
{
    _importerType = type;
}
