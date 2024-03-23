#pragma once

#include "ObjectEntryBase.h"
#include "Object.h"
#include "ISerializeable.h"
#include "MemoryWriter.h"
#include "MemoryReader.h"

template <typename T> requires IsA<T, Object>
class ObjectEntry : public ObjectEntryBase, public ISerializeable
{
public:
    ObjectEntry()
    {
        ObjectType = T::StaticType();
        Object = T::StaticType()->template NewObject<T>();
    }

    ObjectEntry(SharedObjectPtr<T> object)
    {
        ObjectType = T::StaticType();
        Object = object;
    }

    ObjectEntry(const ObjectEntry& other)
    {
        ObjectType = other.ObjectType;
        Object = other.Object;
    }

    ObjectEntry(ObjectEntry&& other) noexcept
    {
        ObjectType = other.ObjectType;
        Object = std::move(other.Object);
    }

    ObjectEntry& operator=(const ObjectEntry& other)
    {
        ObjectType = other.ObjectType;
        Object = other.Object;
        return *this;
    }

    ObjectEntry& operator=(ObjectEntry&& other) noexcept
    {
        ObjectType = other.ObjectType;
        Object = std::move(other.Object);
        return *this;
    }

    bool operator==(const ObjectEntry& rhs) const
    {
        return ObjectType == rhs.ObjectType && Object == rhs.Object;
    }

    ~ObjectEntry() override = default;

    // ISerializeable
public:
    bool Serialize(MemoryWriter& writer) const override
    {
        if (Object == nullptr)
        {
            writer << 0ull;
            return true;
        }
        
        writer << ObjectType->GetID();
            
        return dynamic_cast<ISerializeable*>(Object.get())->Serialize(writer);
    }

    bool Deserialize(MemoryReader& reader) override
    {
        uint64 typeID = 0;
        reader >> typeID;

        ObjectType = TypeRegistry::Get().FindTypeForID(typeID);

        if (ObjectType == nullptr)
        {
            return false;
        }
        
        Object = ObjectType->NewObject<T>();

        return dynamic_cast<ISerializeable*>(Object.get())->Deserialize(reader);
    }
};
