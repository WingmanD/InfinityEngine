#include "EntityList.h"

EntityList::EntityList(const Archetype& type) : _type(type)
{
}

const Archetype& EntityList::GetArchetype() const
{
    return _type;
}
