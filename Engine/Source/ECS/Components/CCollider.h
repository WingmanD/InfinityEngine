#pragma once

#include "BoundingBox.h"
#include "ECS/Components/Component.h"
#include "Math/Transform.h"
#include "MeshCollision.h"
#include <variant>
#include "CCollider.reflection.h"

REFLECTED()
class CCollider : public Component
{
    GENERATED()

public:
    PROPERTY(Edit, Serialize)
    Transform ColliderTransform;

    BoundingBox Bounds;

    PROPERTY(Edit, Serialize)
    std::variant<BoundingBox, MeshCollision> Shape;
};
