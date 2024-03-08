#pragma once

#include "Component.h"
#include "CTransform.reflection.h"

REFLECTED()
class CTransform : public Component
{
    GENERATED()

public:
    // todo
    Vector3 Position;
    
public:
    CTransform() = default;
};
