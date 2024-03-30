#pragma once

#include "Asset.h"
#include "Archetype.h"
#include "Containers/DArray.h"
#include "Components/Component.h"
#include "ObjectEntry.h"
#include "EntityTemplate.reflection.h"

class Entity;

REFLECTED()
class EntityTemplate : public Asset
{
    GENERATED()

public:
    void InitializeEntity(Entity& entity) const;
    
    const DArray<ObjectEntry<Component>>& GetComponentEntries() const;
    const Archetype& GetArchetype() const;

protected:
    virtual void OnPropertyChanged(Name propertyName) override;

private:
    PROPERTY(Serialize, DisplayName = "Components")
    DArray<ObjectEntry<Component>> _componentEntries;

    PROPERTY(Serialize)
    Archetype _archetype;
};
