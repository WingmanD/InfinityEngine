#pragma once

#include "Core.h"
#include "Object.h"
#include "MaterialParameterTypes.reflection.h"

REFLECTED(DataStart = "World")
struct PerPassConstants : public Object
{
    PERPASSCONSTANTS_GENERATED()

public:
    Matrix World;
    Matrix ViewProjection;
    Vector3 CameraPosition;
    Vector3 CameraDirection;
    float Time;
};

REFLECTED(DataStart = "Transform")
struct WidgetPerPassConstants : public Object
{
    WIDGETPERPASSCONSTANTS_GENERATED()

public:
    enum class EWidgetFlags : uint32
    {
        Enabled = 1,
        Hovered = 2,
    };

    Matrix Transform;
    EWidgetFlags Flags;
    float Time;
};
ENABLE_ENUM_OPS(WidgetPerPassConstants::EWidgetFlags)
