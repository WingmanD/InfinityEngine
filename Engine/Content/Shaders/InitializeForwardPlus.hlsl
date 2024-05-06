#include "ShaderCore.hlsl"
#include "Math.hlsl"
#include "ForwardPlusCore.hlsl"

ConstantBuffer<Scene> GScene : register (b0);
RWStructuredBuffer<SimpleFrustum> GFrustums : register (u0);

[numthreads(FORWARD_PLUS_GROUP_DIM, FORWARD_PLUS_GROUP_DIM, 1)]
void CS(uint3 threadID : SV_DispatchThreadID)
{
    const uint2 frustumGrid = uint2(GScene.ScreenSize / FORWARD_PLUS_GROUP_DIM);
    if (threadID.x >= frustumGrid.x || threadID.y >= frustumGrid.y)
    {
        return;
    }

    float4 points[4];
    points[0] = float4(threadID.xy * FORWARD_PLUS_GROUP_DIM, -1.0f, 1.0f);
    points[1] = float4(float2(threadID.x + 1, threadID.y) * FORWARD_PLUS_GROUP_DIM, -1.0f, 1.0f);
    points[2] = float4(float2(threadID.x, threadID.y + 1) * FORWARD_PLUS_GROUP_DIM, -1.0f, 1.0f);
    points[3] = float4(float2(threadID.x + 1, threadID.y + 1) * FORWARD_PLUS_GROUP_DIM, -1.0f, 1.0f);

    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        points[i] = ClipSpaceToViewSpace(ScreenSpaceToClipSpace(points[i].xyz, GScene.ScreenSize),GScene.ProjectionInv);
    }

    const float3 zero = float3(0.0f, 0.0f, 0.0f);

    SimpleFrustum frustum;
    frustum.Planes[0] = MakePlane(zero, points[0].xyz, points[1].xyz);
    frustum.Planes[1] = MakePlane(zero, points[3].xyz, points[2].xyz);
    frustum.Planes[2] = MakePlane(zero, points[2].xyz, points[0].xyz);
    frustum.Planes[3] = MakePlane(zero, points[1].xyz, points[3].xyz);

    GFrustums[threadID.x + threadID.y * frustumGrid.x] = frustum;
}
