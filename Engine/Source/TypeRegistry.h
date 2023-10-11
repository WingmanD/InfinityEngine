#pragma once

#include <unordered_map>
#include <cstdint>
#include "Singleton.h"
#include "Type.h"

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

    void PrintRegisteredTypes() const;

private:
    std::unordered_map<uint64_t, Type*> _typeMap;

private:
    void RegisterType(Type* type);

    template <typename T>
    Type* CreateTypeImpl()
    {
        uint64_t id = Type::CalculatePrimaryID<T>();

        if (Type* existingType = FindTypeForID(id))
        {
            return existingType;
        }

        Type* newType = new Type();
        newType->_id = id;
        newType->_familyID = id;
        newType->_fullID = id;
        newType->_name = Type::GetTypeName<T>();
        newType->_cdo = std::make_unique<T>();

        return newType;
    }
};
