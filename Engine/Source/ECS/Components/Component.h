#pragma once

#include "Object.h"
#include "Component.reflection.h"

class World;

REFLECTED()
class Component : public Object
{
    GENERATED()

public:
    void SetName(Name value, PassKey<World>)
    {
        _componentName = value;
    }

    Name GetName() const
    {
        return _componentName;
    }

private:
    PROPERTY(Edit, Serialize, DisplayName = "Name")
    Name _componentName;
};
