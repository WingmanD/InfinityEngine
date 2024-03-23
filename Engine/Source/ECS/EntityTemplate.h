#pragma once

#include "Asset.h"
#include "Containers/DArray.h"
#include "Components/Component.h"
#include "ObjectEntry.h"
#include "EntityTemplate.reflection.h"

REFLECTED()
class EntityTemplate : public Asset
{
    GENERATED()

public:
    const DArray<ObjectEntry<Component>>& GetComponentEntries() const;

private:
    PROPERTY(Serialize, DisplayName = "Components")
    DArray<ObjectEntry<Component>> _componentEntries;
};
