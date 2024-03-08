#pragma once

#include "Object.h"
#include "Component.reflection.h"

REFLECTED()
class Component : public Object
{
    GENERATED()
    
public:
    explicit Component() = default;
};
