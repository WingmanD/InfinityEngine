#pragma once

#include "Component.h"
#include "Math/Transform.h"
#include "CTransform.reflection.h"

REFLECTED()
class CTransform : public Component
{
    GENERATED()

public:
    PROPERTY(Edit, Serialize, DisplayName = "Transform")
    Transform ComponentTransform;
    
public:
    CTransform() = default;
};
