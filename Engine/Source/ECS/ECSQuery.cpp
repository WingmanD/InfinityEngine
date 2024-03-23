#include "ECSQuery.h"

void ECSQuery::AddEntityList(EntityList* entityList, PassKey<EntityListGraph>)
{
    _entityLists.Add(entityList);
}

void ECSQuery::Clear(PassKey<EntityListGraph>)
{
    _entityLists.Clear();
}

const DArray<EntityList*, 8>& ECSQuery::GetEntityLists() const
{
    return _entityLists;
}
