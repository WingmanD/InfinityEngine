struct PerPassConstants
{
    float4x4 World;
    float4x4 ViewProjection;
    float3 CameraPosition;
    float3 CameraDirection;
    float Time;
};

ConstantBuffer<PerPassConstants> GPerPassConstants: register(b0);

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
    float4 PositionWS : POSITION;
    float3 Normal : NORMAL;
    float4 VertexColor : COLOR;
    float2 UV : UV;
};

VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;

    vOut.PositionWS = mul(float4(vIn.PositionLS, 1.0f), GPerPassConstants.World);
    vOut.PositionCS = mul(float4(vIn.PositionLS, 1.0f), mul(GPerPassConstants.ViewProjection, GPerPassConstants.World));
    vOut.Normal = mul(float4(vIn.Normal, 0.0f), GPerPassConstants.World).xyz;
    vOut.VertexColor = vIn.VertexColor;
    vOut.UV = vIn.UV;

    return vOut;
}

float4 PS(VertexOut pIn) : SV_Target
{
    const float3 lightPosition = float3(2.0f, 1.0f, 3.0f);
    const float3 lightColor = float3(1.0f, 0.0f, 1.0f);
    const float3 lightIntensity = float3(1.0f, 1.0f, 1.0f);

    const float3 ambient = float3(0.1f, 0.1f, 0.1f);
    const float shininess = 32.0f;

    const float3 n = normalize(pIn.Normal);
    const float3 v = normalize(GPerPassConstants.CameraPosition - pIn.PositionWS.xyz);

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

    diffuse *= float3(clamp(sin(GPerPassConstants.Time), 0.25f, 1.0f), clamp(cos(GPerPassConstants.Time), 0.25f, 1.0f), 0.0f);

    float4 color = float4(ambient + diffuse + specular, 1.0f);
    return color;
}
