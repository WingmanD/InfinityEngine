#pragma once

#include "Component.h"
#include "CProjectile.reflection.h"

REFLECTED()
class CProjectile : public Component
{
    GENERATED()
    
public:
    PROPERTY(Edit, Serialize)
    float Speed = 10.0f;
    
    PROPERTY(Edit, Serialize)
    float Damage = 50.0f;
};
