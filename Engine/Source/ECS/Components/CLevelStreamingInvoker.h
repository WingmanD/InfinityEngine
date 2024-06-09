#pragma once

#include "Component.h"
#include "CLevelStreamingInvoker.reflection.h"

REFLECTED()
class CLevelStreamingInvoker : public Component
{
    GENERATED()

public:
    float StreamingDistance = 15.0f;
};
