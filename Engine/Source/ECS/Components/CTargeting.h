#pragma once

#include "AssetPtr.h"
#include "ECS/Components/Component.h"
#include "Math/Transform.h"
#include "CTargeting.reflection.h"

class EntityTemplate;
class Entity;

REFLECTED()
class CTargeting : public Component
{
    GENERATED()

public:
    PROPERTY(Edit, Serialize)
    float Range = 10.0f;
    
    PROPERTY(Edit, Serialize)
    float VisionFOV = 90.0f;
    
    PROPERTY(Edit, Serialize)
    AssetPtr<EntityTemplate> ProjectileTemplate;
    
    PROPERTY(Edit, Serialize)
    float RateOfFire = 1.0f;
    
    PROPERTY(Edit, Serialize)
    Transform ProjectileSpawnOffset;

    // PROPERTY(Edit, Serialize)
    float TargetingDelay = 5.0f;

    float TargetingDelayTimer = 0.0f;

    float TimeSinceLastShot = 0.0f;

    Entity* Target = nullptr;
    uint64 TargetID = 0;
    const Archetype* TargetArchetype;
};
