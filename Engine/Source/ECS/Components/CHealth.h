#pragma once

#include "Component.h"
#include "CHealth.reflection.h"

REFLECTED()
class CHealth : public Component
{
    GENERATED()

public:
    PROPERTY(Edit, Serialize)
    float MaxHealth = 100.0f;

    float Health = MaxHealth;
};
