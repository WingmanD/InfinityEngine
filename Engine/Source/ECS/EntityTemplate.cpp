#include "EntityTemplate.h"

const DArray<ObjectEntry<Component>>& EntityTemplate::GetComponentEntries() const
{
    return _componentEntries;
}
