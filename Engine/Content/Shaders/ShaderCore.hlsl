struct PerPassConstants
{
    float4x4 ViewProjection;
    float3 CameraPosition;
    float3 CameraDirection;
    float Time;
};

struct StaticMeshConstants
{
    float4x4 Transform;
};