#pragma once

#include "Component.h"
#include "Math/Transform.h"
#include "CPointLight.reflection.h"

REFLECTED()
class CPointLight : public Component
{
    GENERATED()
    
public:
    PROPERTY(Edit, Serialize)
    Transform LightTransform;

    PROPERTY(Edit, Serialize)
    float Radius = 1.0f;
    
    PROPERTY(Edit, Serialize)
    Vector3 Color = Vector3::One;
    
    PROPERTY(Edit, Serialize)
    float Intensity = 1.0f;
    
    uint32 LightID = 0;
};
