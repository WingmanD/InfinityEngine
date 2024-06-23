#pragma once

#include "AssetPtr.h"
#include "Component.h"
#include "Math/Transform.h"
#include "CSpawner.reflection.h"

class EntityTemplate;

REFLECTED()
class CSpawner : public Component
{
    GENERATED()
    
public:
    PROPERTY(Edit, Serialize)
    Transform SpawnTransform;
    
    PROPERTY(Edit, Serialize)
    float SpawnRate = 1.0f;

    uint32 SpawnCount = 0;
    uint32 TargetSpawnCount = 10;
    float SpawnTimer = 0.0f;

    PROPERTY(Edit, Serialize)
    AssetPtr<EntityTemplate> SpawnTemplate;
};
