#include "ShaderCore.hlsl"
#include "Math.hlsl"
#include "ForwardPlusCore.hlsl"

struct Input
{
    uint NumLights;
};

ConstantBuffer<Scene> GScene : register (b0);
ConstantBuffer<Input> GInput : register (b1);
StructuredBuffer<SimpleFrustum> GFrustums : register (t0);
StructuredBuffer<PointLight> GPointLights : register(t1);
RWStructuredBuffer<UInt> GCounter : register (u0);

RWStructuredBuffer<Tile> GTile : register (u1);
RWStructuredBuffer<UInt> GIndices : register(u2);

groupshared uint GLightCount;
groupshared uint GStartIndex;
groupshared uint GLightIndices[FORWARD_PLUS_TILE_LIGHTS];

[numthreads(FORWARD_PLUS_GROUP_DIM, FORWARD_PLUS_GROUP_DIM, 1)]
void CS(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    if (groupIndex == 0)
    {
        GLightCount = 0;
    }
    GroupMemoryBarrierWithGroupSync();
    
    for (uint i = groupIndex; i < GInput.NumLights; i += FORWARD_PLUS_GROUP_THREADS)
    {
        if (SphereFrustumIntersection(
                GPointLights[i].Location,
                GPointLights[i].Radius,
                GFrustums[groupIndex])
        )
        {
            uint index;
            InterlockedAdd(GLightCount, 1, index);
            if (index < FORWARD_PLUS_TILE_LIGHTS)
            {
                GLightIndices[index] = i;
            }
        }
    }

    GroupMemoryBarrierWithGroupSync();
    if (groupIndex == 0)
    {
        const uint numGroupsX = ceil((float)GScene.ScreenSize.x / (float)FORWARD_PLUS_GROUP_THREADS);
        const uint tileIndex = groupID.x + groupID.y * numGroupsX;

        const uint lightCount = GLightCount;
        InterlockedAdd(GCounter[0].Value, lightCount, GStartIndex);
        GTile[tileIndex].StartIndex = GStartIndex;
        GTile[tileIndex].LightCount = lightCount;
    }
    GroupMemoryBarrierWithGroupSync();

    for (int i = groupIndex; i < GLightCount; i += FORWARD_PLUS_GROUP_THREADS)
    {
        GIndices[GStartIndex + i].Value = GLightIndices[i];
    }
}
