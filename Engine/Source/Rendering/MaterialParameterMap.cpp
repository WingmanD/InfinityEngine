#include "MaterialParameterMap.h"
#include "MaterialParameterTypes.h"
#include "MemoryReader.h"
#include "MemoryWriter.h"
#include "Object.h"
#include "Type.h"

bool operator<(const MaterialParameterDescriptor& lhs, const MaterialParameterDescriptor& rhs)
{
    return lhs.Name < rhs.Name;
}

MaterialParameterMap::MaterialParameterMap(const MaterialParameterMap& other)
{
    _dataSize = other._dataSize;
    _data = new std::byte[_dataSize];
    std::memset(_data, 0, _dataSize);

    _parameters.Reserve(other._parameters.Count());

    for (const MaterialParameterBinding& binding : other._parameters)
    {
        _parameters.Add(binding);
        MaterialParameterBinding& newBinding = _parameters.Back();
        newBinding.Parameter = nullptr;

        _nameToParameter[binding.Name] = &newBinding;

        if (binding.Parameter != nullptr)
        {
            const size_t offset = reinterpret_cast<std::byte*>(binding.Parameter) - other._data;

            MaterialParameter* newParameter = static_cast<MaterialParameter*>(binding.Parameter->DuplicateAt(_data + offset));
            if (!newParameter->Initialize())
            {
                LOG(L"Failed to initialize material parameter {}!", Util::ToWString(binding.Name));
                DEBUG_BREAK();
                continue;
            }

            newBinding.Parameter = newParameter;
        }
    }

    for (const DefaultParameter& defaultParameter : other._defaultParameters)
    {
        _defaultParameters.Add({
            std::dynamic_pointer_cast<MaterialParameter>(defaultParameter.Parameter->Duplicate()),
            defaultParameter.ParameterName
        });
    }
}

MaterialParameterMap::~MaterialParameterMap()
{
    for (const MaterialParameterBinding& binding : _parameters)
    {
        if (binding.Parameter != nullptr && !binding.ParameterType->GetCDO<MaterialParameter>()->IsShared())
        {
            binding.Parameter->~MaterialParameter();
        }
    }

    delete[] _data;
}

std::unique_ptr<MaterialParameterMap> MaterialParameterMap::Duplicate() const
{
    return std::make_unique<MaterialParameterMap>(*this);
}

bool MaterialParameterMap::Initialize(const std::set<MaterialParameterDescriptor>& parameterDescriptors,
                    const DArray<DefaultMaterialParameterDescriptor, 4>& defaultParameterTypes)
{
    _dataSize = std::ranges::fold_left(parameterDescriptors,
                                       0,
                                       [](size_t size, const MaterialParameterDescriptor& parameterDescriptor)
                                       {
                                           if (parameterDescriptor.ParameterType->GetCDO<MaterialParameter>()->IsShared())
                                           {
                                               return size;
                                           }

                                           return size + parameterDescriptor.ParameterType->GetSize();
                                       });

    _data = new std::byte[_dataSize];
    std::memset(_data, 0, _dataSize);
    size_t offset = 0;

    _parameters.Reserve(parameterDescriptors.size());

    for (const MaterialParameterDescriptor& descriptor : parameterDescriptors)
    {
        if (descriptor.ParameterType->GetCDO<MaterialParameter>()->IsShared())
        {
            _parameters.Add({nullptr, descriptor.ParameterType, descriptor.Name, descriptor.SlotIndex});
            _nameToParameter[descriptor.Name] = &_parameters.Back();
            continue;
        }

        MaterialParameter* newParameter = descriptor.ParameterType->NewObjectAt<MaterialParameter>(_data + offset);
        if (!newParameter->Initialize())
        {
            LOG(L"Failed to initialize material parameter {}!", Util::ToWString(descriptor.Name));
            return false;
        }

        _parameters.Add({newParameter, descriptor.ParameterType, descriptor.Name, descriptor.SlotIndex});
        _nameToParameter[descriptor.Name] = &_parameters.Back();

        offset += descriptor.ParameterType->GetSize();
    }

    for (const DefaultMaterialParameterDescriptor& descriptor : defaultParameterTypes)
    {
        _defaultParameters.Add({descriptor.ParameterType->NewObject<MaterialParameter>(), Name(descriptor.Name)});
    }

    return true;
}

void MaterialParameterMap::ForEachParameterBinding(const std::function<bool(MaterialParameterBinding&)>& func)
{
    for (MaterialParameterBinding& parameter : _parameters)
    {
        if (!func(parameter))
        {
            break;
        }
    }
}

const DArray<MaterialParameterMap::DefaultParameter, 4>& MaterialParameterMap::GetDefaultParameters() const
{
    return _defaultParameters;
}

void MaterialParameterMap::SetSharedParameter(const std::string& name, const SharedObjectPtr<MaterialParameter>& parameter, bool allowMissing /*= false*/)
{
    if (!_nameToParameter.contains(name))
    {
        if (!allowMissing)
        {
            DEBUG_BREAK();
        }

        LOG(L"Failed to find parameter {}!", Util::ToWString(name));
        return;
    }

    // todo refactor this - we must store a weak ptr because we don't own shared parameters

    MaterialParameterBinding* binding = _nameToParameter[name];
    if (parameter != nullptr && binding->ParameterType != parameter->GetType())
    {
        DEBUG_BREAK();
        return;
    }

    binding->Parameter = parameter.get();
}

bool MaterialParameterMap::Serialize(MemoryWriter& writer) const
{
    writer << _nameToParameter.size();

    for (const MaterialParameterBinding& materialParameterBinding : _parameters)
    {
        writer << materialParameterBinding.ParameterType->GetID();
        writer << materialParameterBinding.Name;
        writer << materialParameterBinding.SlotIndex;
    }

    writer << _defaultParameters.Count();
    for (const DefaultParameter& defaultParameter : _defaultParameters)
    {
        writer << defaultParameter.Parameter->GetType()->GetID();
        defaultParameter.Parameter->Serialize(writer);
        writer << defaultParameter.ParameterName;
    } 

    return true;
}

bool MaterialParameterMap::Deserialize(MemoryReader& reader)
{
    size_t size;
    reader >> size;

    std::set<MaterialParameterDescriptor> parameters;

    for (auto i = 0; i < size; ++i)
    {
        uint64 typeID;
        reader >> typeID;

        std::string name;
        reader >> name;

        uint32 slotIndex = 0;
        reader >> slotIndex;

        const Type* type = TypeRegistry::Get().FindTypeForID(typeID);
        if (type == nullptr)
        {
            DEBUG_BREAK();
            return false;
        }

        parameters.insert({type, name, slotIndex});
    }

    uint64 defaultParameterCount;
    reader >> defaultParameterCount;

    for (uint64 i = 0; i < defaultParameterCount; ++i)
    {
        uint64 typeID;
        reader >> typeID;

        Type* type = TypeRegistry::Get().FindTypeForID(typeID);

        DefaultParameter& defaultParameter = _defaultParameters.Emplace();
        defaultParameter.Parameter = type->NewObject<MaterialParameter>();
        defaultParameter.Parameter->Deserialize(reader);

        reader >> defaultParameter.ParameterName;
    }

    return Initialize(parameters, {});
}

const std::unordered_map<std::string, MaterialParameterMap::MaterialParameterBinding*>& MaterialParameterMap::GetNameToParameterMap() const
{
    return _nameToParameter;
}

const DArray<MaterialParameterMap::MaterialParameterBinding, 4>& MaterialParameterMap::GetParameters() const
{
    return _parameters;
}

MaterialParameter* MaterialParameterMap::GetParameter(const std::string& name)
{
    if (!_nameToParameter.contains(name))
    {
        DEBUG_BREAK();
        return nullptr;
    }

    return _nameToParameter[name]->Parameter;
}
