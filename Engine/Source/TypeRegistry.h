#pragma once

#include "Singleton.h"
#include "Type.h"
#include "Util.h"
#include <cstdint>
#include <unordered_map>

class TypeRegistry : public Singleton<TypeRegistry>
{
public:
    template <typename T>
    Type* CreateRootType()
    {
        if (Type* existingType = FindTypeForID(Type::CalculatePrimaryID<T>()))
        {
            return existingType;
        }

        Type* newType = CreateTypeImpl<T>();
        newType->UpdateFullName();

        RegisterType(newType);

        return newType;
    }

    template <typename T, typename... ParentTypes>
    Type* CreateType()
    {
        if (Type* existingType = FindTypeForID(Type::CalculatePrimaryID<T>()))
        {
            return existingType;
        }

        Type* newType = CreateTypeImpl<T>();
        (newType->AddParentType<ParentTypes>(), ...);
        newType->UpdateFullName();

        RegisterType(newType);

        return newType;
    }

    Type* FindTypeForID(uint64_t id);
    Type* FindTypeByName(const std::string& name);

    void LogRegisteredTypes() const;

private:
    std::unordered_map<uint64_t, Type*> _typeMap;
    std::unordered_map<std::string, Type*> _typeNameMap;

private:
    void RegisterType(Type* type);

    template <typename T>
    Type* CreateTypeImpl()
    {
        const uint64_t id = Type::CalculatePrimaryID<T>();

        if (Type* existingType = FindTypeForID(id))
        {
            return existingType;
        }

        Type* newType = new Type();
        newType->_id = id;
        newType->_familyID = id;
        newType->_fullID = id;
        newType->_name = NameOf<T>();
        newType->_cdo = std::make_unique<T>();
        newType->_size = sizeof(T);
        newType->_alignment = alignof(T);
        newType->_alignedSize = Util::AlignedSize(newType->_size, newType->_alignment);

        return newType;
    }
};
