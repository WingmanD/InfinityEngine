#include "Material.h"
#include "Shader.h"
#include "MaterialParameterMap.h"
#include "Engine/Subsystems/AssetManager.h"
#include "Engine/Subsystems/RenderingSubsystem.h"
#include "Rendering/Widgets/TableWidget.h"

IDGenerator<uint32> Material::_materialIDGenerator;
std::unordered_map<uint32, std::weak_ptr<Material>> Material::_materialIDToMaterial;

SharedObjectPtr<Material> Material::GetMaterialByID(uint32 materialID)
{
    return _materialIDToMaterial[materialID].lock();
}

Material::Material(Name name) : Asset(name)
{
}

Material::Material(const Material& other) : Asset(other)
{
    _shader = other._shader;

    if (other._materialParameterMap != nullptr)
    {
        _materialParameterMap = other._materialParameterMap->Duplicate();
    }
}

void Material::SetShader(const SharedObjectPtr<Shader>& shader)
{
    if (_shader == shader)
    {
        return;
    }
    
    _shader = shader;

    OnShaderChanged();
}

SharedObjectPtr<Shader> Material::GetShader() const
{
    return _shader;
}

MaterialParameterMap* Material::GetParameterMap() const
{
    return _materialParameterMap.get();
}

uint32 Material::GetMaterialID() const
{
    return _materialID;
}

MaterialRenderingData* Material::GetRenderingData() const
{
    return _renderingData.get();
}

bool Material::Initialize()
{
    if (!Asset::Initialize())
    {
        return false;
    }

    SubscribeToShaderEvents();

    _renderingData = RenderingSubsystem::Get().CreateMaterialRenderingData();
    _renderingData->SetMaterial(this, {});

    return true;
}

bool Material::Serialize(MemoryWriter& writer) const
{
    if (!Asset::Serialize(writer))
    {
        return false;
    }

    if (_shader == nullptr)
    {
        writer << static_cast<uint64>(0u);
    }
    else
    {
        writer << _shader->GetAssetID();
    }

    _materialParameterMap->Serialize(writer);

    return true;
}

bool Material::Deserialize(MemoryReader& reader)
{
    if (!Asset::Deserialize(reader))
    {
        return false;
    }

    uint64 shaderID;
    reader >> shaderID;

    _shader = AssetManager::Get().FindAsset<Shader>(shaderID);

    _materialParameterMap = std::make_unique<MaterialParameterMap>();
    _materialParameterMap->Deserialize(reader);

    return true;
}

void Material::OnPropertyChanged(Name propertyName)
{
    Asset::OnPropertyChanged(propertyName);

    if (propertyName == Name(L"Shader"))
    {
        OnShaderChanged();
    }
}

void Material::PostLoad()
{
    Asset::PostLoad();

    _materialID = _materialIDGenerator.GenerateID();
    _materialIDToMaterial[_materialID] = SharedFromThis();
}

SharedObjectPtr<Widget> Material::CreateEditWidget()
{
    SharedObjectPtr<Widget> parent = Asset::CreateEditWidget();
    if (parent == nullptr)
    {
        return nullptr;
    }

    if (_materialParameterMap != nullptr)
    {
        SharedObjectPtr<TableWidget> tableWidget = std::dynamic_pointer_cast<TableWidget>(parent);
        if (tableWidget != nullptr)
        {
            const SharedObjectPtr<TableRowWidget> row = NewObject<TableRowWidget>();
            if (row->Initialize())
            {
                row->AddChild(ReflectionWidgets::CreateEditableWidgetFor(SharedFromThis(), _materialParameterMap.get()));
                
                tableWidget->AddRow(row);
            }
        }
    }

    return parent;
}

void Material::OnShaderChanged(const SharedObjectPtr<Shader>& oldShader /*= nullptr*/)
{
    if (oldShader != nullptr)
    {
        oldShader->OnRecompiled.Remove(_materialParameterMapChangedHandle);
    }

    SubscribeToShaderEvents();

    MarkDirtyForAutosave();
}

void Material::SubscribeToShaderEvents()
{
    if (_shader != nullptr)
    {
        _shader->OnRecompiled.Remove(_materialParameterMapChangedHandle);
    }

    if (_shader == nullptr)
    {
        _materialParameterMap = nullptr;
        return;
    }
    else
    {
        std::weak_ptr weakThis = shared_from_this();
        auto func = [weakThis](const Shader* shader)
        {
            const SharedObjectPtr<Material> sharedThis = std::static_pointer_cast<Material>(weakThis.lock());
            if (sharedThis == nullptr)
            {
                return;
            }
            
             std::unique_ptr<MaterialParameterMap> parameterMap = shader->CreateMaterialParameterMap();
            
             if (sharedThis->_materialParameterMap != nullptr)
             {
                 // todo copy over constant parameters from old map to new map
                 for (const MaterialParameterMap::DefaultParameter& defaultParameter : sharedThis->_materialParameterMap->GetDefaultParameters())
                 {
                     // todo refactor
                     DArray<MaterialParameterMap::DefaultParameter, 4>& defaults =
                         const_cast<DArray<MaterialParameterMap::DefaultParameter, 4>&>(parameterMap->
                             GetDefaultParameters());
                     auto it = defaults.FindIf(
                         [&defaultParameter](const MaterialParameterMap::DefaultParameter& parameter)
                         {
                             return parameter.ParameterName == defaultParameter.ParameterName;
                         });
            
                     if (it != defaults.end())
                     {
                         defaults[it.GetIndex()] = defaultParameter;
                     }
                 }
             }

            sharedThis->_materialParameterMap = std::move(parameterMap);
        };
        _materialParameterMapChangedHandle = _shader->OnRecompiled.Add(func);

        if (_materialParameterMap == nullptr)
        {
            func(_shader.Get().get());
        }
    }
}
