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
        writer << 0ull;
    }

    if (ParameterMap != nullptr)
    {
        writer << true;
        ParameterMap->Serialize(writer);
    }
    else
    {
        writer << false;
    }

    writer << _lastCompileTime;

    return true;
}

bool Shader::Deserialize(MemoryReader& reader)
{
    if (!Asset::Deserialize(reader))
    {
        return false;
    }

    bool success = true;

    uint64 typeID;
    reader >> typeID;

    MaterialInstanceDataType = TypeRegistry::Get().FindTypeForID(typeID);

    bool hasParameterMap;
    reader >> hasParameterMap;

    if (hasParameterMap)
    {
        ParameterMap = std::make_unique<MaterialParameterMap>();
        success &= ParameterMap->Deserialize(reader);
    }

    reader >> _lastCompileTime;

    return success;
}

void Shader::SetLastCompileTime(std::filesystem::file_time_type time)
{
    _lastCompileTime = time;
}

std::filesystem::file_time_type Shader::GetLastCompileTime() const
{
    return _lastCompileTime;
}
