#pragma once

#include "ECS/Components/Component.h"
#include "ECS/Components/CFloatingControl.reflection.h"

REFLECTED()
class CFloatingControl : public Component
{
    GENERATED()
    
public:
    PROPERTY(Edit, Serialize)
    float Speed = 1.0f;

    PROPERTY(Edit, Serialize)
    float AngularSpeed = 5.0f;
};
