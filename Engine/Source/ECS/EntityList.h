#pragma once

#include "Archetype.h"
#include "Entity.h"
#include "Containers/BucketArray.h"

class EntityList : public BucketArray<Entity>
{
public:
    explicit EntityList() = default;
    explicit EntityList(const Archetype& type);

    const Archetype& GetArchetype() const;
    
private:
    Archetype _type;
};
