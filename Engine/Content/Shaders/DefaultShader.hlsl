#include "ShaderCore.hlsl"

StructuredBuffer<SMInstance> GInstanceBuffer: register(t0);
ConstantBuffer<Scene> GScene: register(b0);

struct VertexIn
{
    float3 PositionLS : POSITION;
    float3 Normal : NORMAL;
    float4 VertexColor : COLOR;
    float2 UV : UV;
};

struct VertexOut
{
    float4 PositionCS : SV_POSITION;
    nointerpolation float4 PositionWS : POSITION;
    nointerpolation float3 Normal : NORMAL;
    float4 VertexColor : COLOR;
    float2 UV : UV;
    uint InstanceID : SV_InstanceID;
};

VertexOut VS(VertexIn vIn, uint instanceID : SV_InstanceID)
{
    VertexOut vOut;
    
    vOut.PositionWS = mul(float4(vIn.PositionLS, 1.0f), GInstanceBuffer[instanceID].World);
    vOut.PositionCS = mul(vOut.PositionWS, GScene.ViewProjection);
    vOut.Normal = mul(float4(vIn.Normal, 0.0f), GInstanceBuffer[instanceID].World).xyz;
    vOut.VertexColor = vIn.VertexColor;
    vOut.UV = vIn.UV * GInstanceBuffer[instanceID].MaterialIndex;
    vOut.InstanceID = instanceID;

    return vOut;
}

float4 PS(VertexOut pIn) : SV_Target
{
    const float3 lightPosition = float3(2.0f, 1.0f, 3.0f);
    const float3 lightColor = float3(1.0f, 1.0f, 1.0f);
    const float3 lightIntensity = float3(1.0f, 1.0f, 1.0f) * GInstanceBuffer[pIn.InstanceID].MaterialIndex + 0.25f;

    const float3 ambient = float3(0.1f, 0.1f, 0.1f);
    const float shininess = 32.0f;
    
    const float3 n = normalize(pIn.Normal);
    const float3 v = normalize(GScene.CameraPosition - pIn.PositionWS.xyz);

    float3 diffuse = 0.0f;
    float3 specular = 0.0f;

    float3 lOriginal = lightPosition - pIn.PositionWS.xyz;

    const float kc = 1.0;
    const float kl = 0.09;
    const float kq = 0.032;

    float dl = length(lOriginal);

    float denominator = kc + kl * dl + kq * dl * dl;

    float3 l = normalize(lOriginal);
    //float3 r = reflect(-l, n);

    float3 h = normalize(l + v);

    diffuse += lightIntensity * lightColor * max(dot(l, n), 0) / denominator;
    specular += lightIntensity * lightColor * pow(max(dot(h, n), 0), shininess) / denominator;

    diffuse *= float3(sin(GScene.Time % 1.0f), cos(GScene.Time % 1.0f), 0.0f);

    float4 color = float4(ambient + diffuse + specular, 1.0f);
    return color;
}
