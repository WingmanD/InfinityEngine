#include "Material.h"
#include "Shader.h"
#include "Engine/Subsystems/AssetManager.h"

Material::Material(const std::wstring& name) : Asset(name)
{
}

void Material::SetShader(const std::shared_ptr<Shader>& shader)
{
    _shader = shader;

    MarkDirtyForAutosave();
}

std::shared_ptr<Shader> Material::GetShader() const
{
    return _shader;
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
