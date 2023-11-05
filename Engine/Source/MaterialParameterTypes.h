#pragma once

#include "Object.h"
#include "Math/Vector3.h"
#include <DirectXMath.h>
#include "MaterialParameterTypes.reflection.h"

REFLECTED(DataStart = "World")
struct PerPassConstants : public Object
{
    PERPASSCONSTANTS_GENERATED()

public:
    DirectX::XMFLOAT4X4 World;
    DirectX::XMFLOAT4X4 ViewProjection;
    Vector3 CameraPosition;
    Vector3 CameraDirection;
    float Time;
};
