#pragma once

#include "Component.h"
#include "Math/Transform.h"
#include "ECS/World.h"
#include "CTransform.reflection.h"

REFLECTED()
class CTransform : public Component
{
    GENERATED()
    
public:
    static constexpr auto OnChanged = &World::OnTransformChanged;

    PROPERTY(Edit, Serialize, DisplayName = "Transform")
    Transform ComponentTransform;
    
public:
    CTransform() = default;
};
