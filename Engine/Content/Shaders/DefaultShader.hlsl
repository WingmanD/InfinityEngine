struct PerPassConstants
{
    float4x4 World; 
    float4x4 ViewProjection;
    float Time;
};

ConstantBuffer<PerPassConstants> GPerPassConstants: register(b0);

struct VertexIn
{
    float3 PosLS : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosCS  : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;
	
    vOut.PosCS = mul(float4(vIn.PosLS, 1.0f), mul(GPerPassConstants.ViewProjection, GPerPassConstants.World));
    
    vOut.Color = float4(clamp(sin(GPerPassConstants.Time), 0.25f, 1.0f), clamp(cos(GPerPassConstants.Time), 0.25f, 1.0f), 0.0f, 1.0f);
    
    return vOut;
}

float4 PS(VertexOut pIn) : SV_Target
{
    return pIn.Color;
}