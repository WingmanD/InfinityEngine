#include "MaterialParameterMap.h"
#include "Type.h"
#include "Object.h"
#include "MemoryWriter.h"
#include "MemoryReader.h"
#include <ranges>

bool operator<(const MaterialParameter& lhs, const MaterialParameter& rhs)
{
    return lhs.Name < rhs.Name;
}

MaterialParameterMap::MaterialParameterMap(const MaterialParameterMap& other)
{
    _dataSize = other._dataSize;
    _data = new std::byte[_dataSize];
    std::memcpy(_data, other._data, _dataSize);

    for (const auto& [name, object] : other._nameToObject)
    {
        const size_t offset = reinterpret_cast<std::byte*>(object) - other._data;
        _nameToObject[name] = object->DuplicateAt(_data + offset);
    }
}

MaterialParameterMap::~MaterialParameterMap()
{
    delete[] _data;
}

std::unique_ptr<MaterialParameterMap> MaterialParameterMap::Duplicate() const
{
    return std::make_unique<MaterialParameterMap>(*this);
}

bool MaterialParameterMap::Initialize(const std::set<MaterialParameter>& parameterDescriptors)
{
    _dataSize = std::ranges::fold_left(parameterDescriptors,
                                       0,
                                       [](size_t size, const MaterialParameter& parameterDescriptor)
                                       {
                                           return size + parameterDescriptor.ParameterType->GetAlignedSize();
                                       });

    _data = new std::byte[_dataSize];
    size_t offset = 0;

    for (const MaterialParameter& descriptor : parameterDescriptors)
    {
        _nameToObject[descriptor.Name] = descriptor.ParameterType->NewObjectAt(_data + offset);
        offset += descriptor.ParameterType->GetAlignedSize();
    }

    return true;
}

bool MaterialParameterMap::Serialize(MemoryWriter& writer) const
{
    writer << _nameToObject.size();
    for (const auto& [name, object] : _nameToObject)
    {
        writer << name;
        writer << object->GetType()->GetID();
    }

    return true;
}

bool MaterialParameterMap::Deserialize(MemoryReader& reader)
{
    size_t size;
    reader >> size;

    std::set<MaterialParameter> parameters;

    for (auto i = 0; i < size; ++i)
    {
        std::string name;
        reader >> name;

        uint64 typeID;
        reader >> typeID;

        const Type* type = TypeRegistry::Get().FindTypeForID(typeID);
        if (type == nullptr)
        {
            DEBUG_BREAK();
            return false;
        }

        parameters.insert({name, type, 0});
    }

    return Initialize(parameters);
}

const std::unordered_map<std::string, Object*>& MaterialParameterMap::GetNameToObjectMap()
{
    return _nameToObject;
}
