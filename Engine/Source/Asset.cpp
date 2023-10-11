#include "Asset.h"

Asset::Asset(std::string name) : _name(std::move(name))
{
    _id = std::hash<std::string>{}(_name);
}

Asset::Asset(std::string name, std::filesystem::path importPath) : _name(std::move(name)), _importPath(std::move(importPath))
{
    _id = std::hash<std::string>{}(_name);
}

bool Asset::Serialize(MemoryWriter& writer)
{
    writer << StaticType()->GetID();
    writer << _id;
    writer << _name;

    return true;
}

bool Asset::Deserialize(MemoryReader& reader)
{
    reader >> _id;
    reader >> _name;

    return true;
}

uint64_t Asset::GetAssetID() const
{
    return _id;
}

const std::string& Asset::GetName() const
{
    return _name;
}
