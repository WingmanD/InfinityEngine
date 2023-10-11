#include "TypeRegistry.h"
#include "Type.h"
#include "Object.h"

Type *TypeRegistry::FindTypeForID(uint64_t id) {
    if (_typeMap.contains(id)) {
        return _typeMap[id];
    }

    return nullptr;
}

void TypeRegistry::RegisterType(Type *type) {
    if (_typeMap.contains(type->GetID())) {
        return;
    }

    _typeMap[type->GetID()] = type;
}

void TypeRegistry::PrintRegisteredTypes() const {
    std::cout << "All registered types: " << std::endl;
    for (const auto &[id, type]: _typeMap) {
        std::cout << "ID: " << id << ", Full Name: " << type->GetFullName() << std::endl;
    }
}
