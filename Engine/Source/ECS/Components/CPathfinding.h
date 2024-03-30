#pragma once

#include "Component.h"
#include "CPathfinding.reflection.h"

REFLECTED()
class CPathfinding : public Component
{
    GENERATED()
    
public:
    Vector3 Destination;
    float Speed = 1.0f;
};
