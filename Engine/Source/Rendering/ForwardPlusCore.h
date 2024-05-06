#pragma once

#include "CoreMinimal.h"
#include "Math/MathFwd.h"

enum ERenderingSettings : uint32
{
    ForwardPlusTileDim = 16,
    ForwardPlusTileMaxLights = 256
};

struct SimpleFrustum
{
    Vector4 Planes[4];
};

struct Tile
{
    uint32 StartIndex;
    uint32 LightCount;
};
