#pragma once

#include "Component.h"
#include "CLevelStreamingInvoker.reflection.h"

REFLECTED()
class CLevelStreamingInvoker : public Component
{
    GENERATED()

public:
    PROPERTY(Edit, Serialize)
    float StreamingDistance = 15.0f;
};
