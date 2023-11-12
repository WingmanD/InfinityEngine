#define EWidgetFlags uint
#define Enabled 1
#define Hovered 2

struct WidgetPerPassConstants
{
    float4x4 Transform;
    EWidgetFlags Flags;
    float Time;
};

ConstantBuffer<WidgetPerPassConstants> GWidgetConstants : register(b0);

struct VertexIn
{
    float3 PositionLS : POSITION;
    float3 Normal : NORMAL;
    float4 VertexColor : COLOR;
    float2 UV : UV;
};

struct VertexOut
{
    float4 PositionSS : SV_POSITION;
    float4 PositionCS : POSITION_CS;
    float4 VertexColor : COLOR;
    float2 UV : UV;
};

VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;

    vOut.PositionSS = mul(GWidgetConstants.Transform, float4(vIn.PositionLS, 1.0f));
    vOut.PositionCS = vOut.PositionSS;
    vOut.VertexColor = vIn.VertexColor;
    vOut.UV = vIn.UV;

    return vOut;
}

bool HasFlag(EWidgetFlags flag)
{
    return GWidgetConstants.Flags & flag;
}

float2 GetWidgetPositionCS()
{
    return float2(GWidgetConstants.Transform[0][3], GWidgetConstants.Transform[1][3]);
}

float GetWidgetRotation()
{
    return atan2(GWidgetConstants.Transform[1][0], GWidgetConstants.Transform[1][1]);
}

float2 GetWidgetScale()
{
    float sx = length(float2(GWidgetConstants.Transform[0][0], GWidgetConstants.Transform[1][0]));
    float sy = length(float2(GWidgetConstants.Transform[0][1], GWidgetConstants.Transform[1][1]));
    return float2(sx, sy);
}

float Box2D(float2 position, float2 size, float radius)
{
    position = abs(position) - size + radius;
    return length(max(position, 0.0)) + min(max(position.x, position.y), 0.0) - radius;
}

float4 PS(VertexOut pIn) : SV_Target
{
    float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);

    // todo rotation
    const float2 position = GetWidgetPositionCS();
    const float rotation = GetWidgetRotation();
    const float2 scale = GetWidgetScale();
    
    const float radius = 0.01f;
    const float distance = Box2D(pIn.PositionCS.xy - position, scale / 2.0f, radius);

    color = color * distance < 0.0f;

    if (!HasFlag(Enabled))
    {
        return color * float4(0.5f, 0.5f, 0.5f, 1.0f);
    }

    if (HasFlag(Hovered))
    {
        return color * float4(1.25f, 1.25f, 1.25f, 1.0f);
    }

    return color;
}
