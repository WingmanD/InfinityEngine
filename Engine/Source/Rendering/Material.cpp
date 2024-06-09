#include "Material.h"
#include "Shader.h"
#include "MaterialParameterMap.h"
#include "Engine/Subsystems/AssetManager.h"
#include "Engine/Subsystems/RenderingSubsystem.h"

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
    
    if (_shader != nullptr)
    {
        _materialParameterMap = _shader->CreateMaterialParameterMap();
    }

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

void Material::OnShaderChanged(const SharedObjectPtr<Shader>& oldShader /*= nullptr*/)
{
    if (oldShader != nullptr)
    {
        oldShader->OnRecompiled.Remove(_materialParameterMapChangedHandle);
    }

    if (_shader == nullptr)
    {
        _materialParameterMap = nullptr;
        return;
    }
    else
    {
        std::weak_ptr weakThis = shared_from_this();
        _materialParameterMapChangedHandle = _shader->OnRecompiled.Add([weakThis](const Shader* shader)
        {
            const SharedObjectPtr<Material> sharedThis = std::static_pointer_cast<Material>(weakThis.lock());
            if (sharedThis == nullptr)
            {
                return;
            }

            // todo copy over parameters from old map to new map
            sharedThis->_materialParameterMap = shader->CreateMaterialParameterMap();
        });

        _materialParameterMap = _shader->CreateMaterialParameterMap();
    }

    MarkDirtyForAutosave();
}
