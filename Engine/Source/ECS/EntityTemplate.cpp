#include "EntityTemplate.h"

void EntityTemplate::InitializeEntity(Entity& entity) const
{
    for (int32 i = 0; i < _componentEntries.Count(); ++i)
    {
        Component* component = entity.Get(i);
        *component = *_componentEntries[i];
    }
}

const DArray<ObjectEntry<Component>>& EntityTemplate::GetComponentEntries() const
{
    return _componentEntries;
}

const Archetype& EntityTemplate::GetArchetype() const
{
    return _archetype;
}

void EntityTemplate::OnPropertyChanged(Name propertyName)
{
    Asset::OnPropertyChanged(propertyName);

    if (propertyName == Name(L"Components"))
    {
        _archetype = Archetype();

        for (ObjectEntry<Component>& componentEntry : _componentEntries)
        {
            _archetype.AddComponent(*componentEntry);
        }
    }
}
