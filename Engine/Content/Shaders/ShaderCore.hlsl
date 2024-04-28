struct Scene
{
    float4x4 ViewProjection;
    float3 CameraLocationWS;
    float Time;
    float3 CameraForward;
    float HorizontalFOV;
    float3 CameraUp;
    float VerticalFOV;
    float DrawDistance;
};

struct SMInstance
{
    float4x4 World;
    uint MeshID;
    uint LOD;
    uint MaterialID;
    uint MaterialIndex;
    uint Count;
};

struct AABB
{
    float3 Min;
    float3 Max;
};

struct MeshInfo
{
    AABB BoundingBox;
};
