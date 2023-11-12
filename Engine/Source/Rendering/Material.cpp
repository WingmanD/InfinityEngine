#include "Material.h"
#include "Shader.h"
#include "MaterialParameterMap.h"
#include "Engine/Subsystems/AssetManager.h"
#include "Engine/Subsystems/RenderingSubsystem.h"

Material::Material(const std::wstring& name) : Asset(name)
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

void Material::SetShader(const std::shared_ptr<Shader>& shader)
{
    if (_shader == shader)
    {
        return;
    }

    if (_shader != nullptr)
    {
        _shader->OnRecompiled.Unsubscribe(_materialParameterMapChangedHandle);
    }

    _shader = shader;

    if (_shader == nullptr)
    {
        _materialParameterMap = nullptr;
        return;
    }

    OnShaderChanged();

    MarkDirtyForAutosave();
}

std::shared_ptr<Shader> Material::GetShader() const
{
    return _shader;
}

MaterialParameterMap& Material::GetParameterMap() const
{
    return *_materialParameterMap.get();
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

void Material::OnShaderChanged()
{
    std::weak_ptr weakThis = shared_from_this();
    _materialParameterMapChangedHandle = _shader->OnRecompiled.Subscribe([weakThis](const Shader* shader)
    {
        std::shared_ptr<Material> sharedThis = std::static_pointer_cast<Material>(weakThis.lock());
        if (sharedThis == nullptr)
        {
            return;
        }

        // todo copy over parameters from old map to new map
        sharedThis->_materialParameterMap = shader->CreateMaterialParameterMap();
    });

    _materialParameterMap = _shader->CreateMaterialParameterMap();
}
