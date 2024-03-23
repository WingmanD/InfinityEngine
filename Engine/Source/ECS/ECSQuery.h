#pragma once

#include "PassKey.h"
#include "Containers/DArray.h"

class EntityListGraph;
class EntityList;

class ECSQuery
{
public:
    explicit ECSQuery() = default;

    void AddEntityList(EntityList* entityList, PassKey<EntityListGraph>);
    void Clear(PassKey<EntityListGraph>);

    const DArray<EntityList*, 8>& GetEntityLists() const;
    
private:
    DArray<EntityList*, 8> _entityLists;
};
