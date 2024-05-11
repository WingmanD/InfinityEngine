#include "DirtyTracker.h"

void DirtyTracker::Register(Entity* entity)
{
    const auto result = _dirtyEntities.insert(entity);
    if (result.second)
    {
        _dirtyEntitiesArray.Add(entity);
    }
}

const DArray<Entity*>& DirtyTracker::GetDirtyEntities() const
{
    return _dirtyEntitiesArray;
}

void DirtyTracker::Clear()
{
    _dirtyEntities.clear();
    _dirtyEntitiesArray.Clear();
}
