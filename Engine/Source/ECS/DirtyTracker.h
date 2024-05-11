#pragma once

#include <unordered_set>
#include "Containers/DArray.h"

class Entity;

class DirtyTracker
{
public:
    void Register(Entity* entity);

    const DArray<Entity*>& GetDirtyEntities() const;

    void Clear();

private:
    std::unordered_set<Entity*> _dirtyEntities;
    DArray<Entity*> _dirtyEntitiesArray;
};
