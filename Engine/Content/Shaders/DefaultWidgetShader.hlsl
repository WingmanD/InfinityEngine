#include "WidgetShaderLibrary.hlsl"

ConstantBuffer<WidgetPerPassConstants> GWidgetConstants : register(b0);
ConstantBuffer<WindowGlobals> GWindowGlobals : register(b1);

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
    vOut.PositionSS.x /= GWindowGlobals.AspectRatio;

    vOut.PositionCS = vOut.PositionSS;
    vOut.VertexColor = vIn.VertexColor;
    vOut.UV = vIn.UV;

    return vOut;
}

float4 PS(VertexOut pIn) : SV_Target
{
    float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);

    // const float2 position = GetWidgetPositionCS(GWidgetConstants);
    // const float rotation = GetWidgetRotation(GWidgetConstants);
    // const float2 scale = GetWidgetScale(GWidgetConstants);
    //
    // const float radius = 0.25f;
    // const float border = 0.01f;
    //
    // const float2 boxSize = float2(scale.x * 16.0f / 9.0f, scale.y) / 2.0f;
    //
    // color = color * Box2DWithBorder(pIn.PositionCS.xy, position, rotation, boxSize * 0.75f, radius, border);
    // color *= GWindowGlobals.AspectRatio;
    //
    // if (!HasFlag(GWidgetConstants, Enabled))
    // {
    //     return color * float4(0.5f, 0.5f, 0.5f, 1.0f);
    // }
    //
    // if (HasFlag(GWidgetConstants, Hovered))
    // {
    //     return color * float4(1.25f, 1.25f, 1.25f, 1.0f);
    // }

    return color;
}
