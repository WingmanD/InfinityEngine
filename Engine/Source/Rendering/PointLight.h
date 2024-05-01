#pragma once

#include "Math/MathFwd.h"

struct PointLight
{
    Vector3 Location = Vector3::Zero;
    float Radius = 1.0f;
    Vector3 Color = Vector3::One;
    float Intensity = 1.0f;
};
