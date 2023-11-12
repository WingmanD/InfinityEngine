#define EWidgetFlags uint
#define Enabled 1
#define Hovered 2

struct WidgetPerPassConstants
{
    float4x4 Transform;
    EWidgetFlags Flags;
    float Time;
};

bool HasFlag(WidgetPerPassConstants widgetConstants, EWidgetFlags flag)
{
    return widgetConstants.Flags & flag;
}

float2 GetWidgetPositionCS(WidgetPerPassConstants widgetConstants)
{
    return float2(widgetConstants.Transform[0][3], widgetConstants.Transform[1][3]);
}

float GetWidgetRotation(WidgetPerPassConstants widgetConstants)
{
    return atan2(widgetConstants.Transform[1][0], widgetConstants.Transform[1][1]);
}

float2 GetWidgetScale(WidgetPerPassConstants widgetConstants)
{
    float sx = length(float2(widgetConstants.Transform[0][0], widgetConstants.Transform[1][0]));
    float sy = length(float2(widgetConstants.Transform[0][1], widgetConstants.Transform[1][1]));
    return float2(sx, sy);
}

float Box2D(float2 position, float2 center, float rotation, float2 size, float radius)
{
    position = position - center;
    position = mul(float2x2(cos(rotation), sin(rotation), -sin(rotation), cos(rotation)), position);
    position = abs(position) - size + radius;
    return length(max(position, 0.0)) + min(max(position.x, position.y), 0.0) - radius;
}