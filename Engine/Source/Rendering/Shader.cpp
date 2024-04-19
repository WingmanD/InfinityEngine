#include "Shader.h"
#include "MaterialParameterMap.h"

Shader::Shader(Name name) : Asset(name)
{
}

Shader::Shader(const Shader& other) : Asset(other)
{
}

bool Shader::Recompile(bool immediate)
{
    return true;
}

std::unique_ptr<MaterialParameterMap> Shader::CreateMaterialParameterMap() const
{
    if (ParameterMap == nullptr)
    {
        return nullptr;
    }

    return ParameterMap->Duplicate();
}

Type* Shader::GetMaterialInstanceDataType() const
{
    return MaterialInstanceDataType;
}

bool Shader::Serialize(MemoryWriter& writer) const
{
    if (!Asset::Serialize(writer))
    {
        return false;
    }

    if (MaterialInstanceDataType != nullptr)
    {
        writer << MaterialInstanceDataType->GetID();
    }
    else
    {
        writer << static_cast<uint64>(0);
    }

    return true;
}

bool Shader::Deserialize(MemoryReader& reader)
{
    if (!Asset::Deserialize(reader))
    {
        return false;
    }

    uint64 typeID;
    reader >> typeID;

    MaterialInstanceDataType = TypeRegistry::Get().FindTypeForID(typeID);
    
    return true;
}
