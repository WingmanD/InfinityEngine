#include "ForwardPlusCore.hlsl"
#include "ShaderCore.hlsl"

struct DefaultMaterialParameter
{
    float4 BaseColor;
    float4 EmissiveColor;
    float4 SpecularColor;
    float SpecularPower;
    float Metallic;
};

ConstantBuffer<InstanceOffset> GInstanceOffset : register(b0);
StructuredBuffer<SMInstance> GInstanceBuffer : register(t0);
StructuredBuffer<DefaultMaterialParameter> GMP_DefaultMaterialParameter : register(t1);
StructuredBuffer<PointLight> GPointLights : register(t2);
StructuredBuffer<Tile> GLightTiles : register(t3);
StructuredBuffer<UInt> GLightIndices : register(t4);
ConstantBuffer<Scene> GScene : register(b1);

struct VertexIn
{
    float3 PositionLS : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float4 VertexColor : COLOR;
    float2 UV : UV;
    uint InstanceID : SV_InstanceID;
};

struct VertexOut
{
    float4 PositionCS : SV_POSITION;
    float4 PositionWS : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float4 VertexColor : COLOR;
    float2 UV : UV;
    uint InstanceID : InstanceID;
};

VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;

    const uint instanceID = vIn.InstanceID + GInstanceOffset.Offset;

    vOut.PositionWS = mul(float4(vIn.PositionLS, 1.0f), GInstanceBuffer[instanceID].World);
    vOut.PositionCS = mul(vOut.PositionWS, GScene.ViewProjection);

    vOut.Normal = mul(float4(vIn.Normal, 0.0f), GInstanceBuffer[instanceID].World).xyz;
    vOut.Tangent = mul(float4(vIn.Tangent, 0.0f), GInstanceBuffer[instanceID].World).xyz;
    vOut.Bitangent = mul(float4(vIn.Bitangent, 0.0f), GInstanceBuffer[instanceID].World).xyz;

    vOut.VertexColor = vIn.VertexColor;
    vOut.UV = vIn.UV * GInstanceBuffer[instanceID].MaterialIndex;

    vOut.InstanceID = instanceID;

    return vOut;
}

float3 GGX(float3 n, float3 m, float3 tangent, float3 bitangent, float roughness, float aniso)
{
    const float rSq = pow(roughness, 2);
    const float alphaX = rSq * (1.0f + aniso);
    const float alphaY = rSq * (1.0f - aniso);

    const float nDotM = dot(n, m);

    const float denominator =
        pow(dot(tangent, m), 2) / pow(alphaX, 2) +
        pow(dot(bitangent, m), 2) / pow(alphaY, 2) +
        pow(nDotM, 2.0f);

    return saturate(nDotM) / (3.14f * alphaX * alphaY * pow(denominator, 2));
}

float3 Fresnel(float3 n, float3 l, float f0)
{
    return f0 + (1.0f - f0) * pow(1.0f - dot(n, l), 5.0f);
}

float3 G1(float3 n, float3 s, float alpha)
{
    const float nDotS = dot(n, s);
    return 2.0f * nDotS / (nDotS * (2.0f - alpha) + alpha);
}

float3 CookTorranceBRDF(float3 l, float3 v,
                        float3 n, float3 tangent, float3 bitangent,
                        float f0, float roughness, float aniso)
{
    const float3 h = normalize(l + v);
    const float alpha = pow(roughness, 2);
    return GGX(h, l, tangent, bitangent, roughness, aniso) * Fresnel(l, h, f0) * G1(n, l, alpha) /
        (4.0f * dot(n, l) * dot(n, v));
}

float4 PS(VertexOut pIn) : SV_Target
{
    const float3 ambient = float3(0.1f, 0.1f, 0.1f);
    const float specular = GMP_DefaultMaterialParameter[GInstanceBuffer[pIn.InstanceID].MaterialIndex].SpecularPower;
    //const float roughness = 1.0f - specular / 1000.0f;
    const float roughness = 1.0f;
    const float3 diffuse = GMP_DefaultMaterialParameter[GInstanceBuffer[pIn.InstanceID].MaterialIndex].BaseColor.xyz;

    const float f0 = 0.04f;
    const float aniso = 0.0f;

    const float3 n = normalize(pIn.Normal);
    const float3 v = normalize(GScene.CameraLocationWS - pIn.PositionWS.xyz);

    float3 color = float3(0.0f, 0.0f, 0.0f);

    const uint2 tileIndex = uint2(floor(pIn.PositionCS.xy / FORWARD_PLUS_GROUP_THREADS));
    
    const Tile tile = GLightTiles[tileIndex.x + tileIndex.y];
    for (uint i = tile.StartIndex; i < tile.StartIndex + tile.LightCount; ++i)
    {
        const uint lightIndex = GLightIndices[i].Value;
        const float3 l = normalize(GPointLights[lightIndex].Location - pIn.PositionWS.xyz);
    
        const float3 contribution = GPointLights[lightIndex].Color * GPointLights[lightIndex].Intensity *
            CookTorranceBRDF(l, v, n, pIn.Tangent, pIn.Bitangent, f0, roughness, aniso);
        color += max(0.0f, contribution);
    }
    
    {
        // Directional light
        const float3 directionalLightDir = normalize(float3(0.0f, 0.0f, 1.0f));
        const float directionalLightIntensity = 10.0f;
    
        const float3 contribution = directionalLightIntensity *
            CookTorranceBRDF(directionalLightDir, v, n, pIn.Tangent, pIn.Bitangent, f0, roughness, aniso);
        color += max(0.0f, contribution);
    }

    color *= diffuse;
    color += ambient * diffuse;

    return float4(color, 1.0f);
}
