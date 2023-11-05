#include "TypeRegistry.h"
#include "Type.h"
#include "Object.h"

Type* TypeRegistry::FindTypeForID(uint64_t id)
{
    if (_typeMap.contains(id))
    {
        return _typeMap[id];
    }

    return nullptr;
}

Type* TypeRegistry::FindTypeByName(const std::string& name)
{
    if (!_typeNameMap.contains(name))
    {
        return nullptr;
    }

    return _typeNameMap[name];
}

void TypeRegistry::RegisterType(Type* type)
{
    if (_typeMap.contains(type->GetID()))
    {
        return;
    }

    _typeMap[type->GetID()] = type;
    _typeNameMap[type->GetName()] = type;
}

void TypeRegistry::PrintRegisteredTypes() const
{
    std::cout << "All registered types: " << std::endl;
    for (const auto& [id, type] : _typeMap)
    {
        std::cout << "ID: " << id << ", Full Name: " << type->GetFullName() << std::endl;
    }
}
