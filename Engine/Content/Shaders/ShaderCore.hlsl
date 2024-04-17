struct Scene
{
    float4x4 ViewProjection;
    float3 CameraPosition;
    float3 CameraDirection;
    float Time;
};

struct SMInstance
{
    float4x4 World;
    uint MeshID;
    uint MaterialID;
    uint MaterialIndex;
};