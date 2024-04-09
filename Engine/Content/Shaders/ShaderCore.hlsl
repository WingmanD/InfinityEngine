struct PerPassConstants
{
    float4x4 ViewProjection;
    float3 CameraPosition;
    float3 CameraDirection;
    float Time;
};

struct SMInstance
{
    float4x4 World;
    uint64_t MeshID;
    uint64_t MaterialID;
    uint MaterialIndex;
};