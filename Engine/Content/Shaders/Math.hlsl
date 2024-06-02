static const float PI = 3.14159265359f;
static const float e = 2.71828182846f;

inline float3 VectorPlaneProjection(float3 vec, float3 planeNormal)
{
    return vec - planeNormal * dot(vec, planeNormal);
}

inline float DistanceFromPlane(float3 plane, float3 planeNormal, float3 planePoint)
{
    return abs(dot(planeNormal, plane) - dot(planeNormal, planePoint)) / length(planeNormal);
}

inline float DistanceFromPlane(float3 p, float3 planeNormal, float planeD)
{
    return -(dot(planeNormal, p) - planeD);
}

inline float Angle(float3 a, float3 b)
{
    return acos(dot(a, b) / (length(a) * length(b)));
}

inline float AngleFast(float3 a, float3 b)
{
    return acos(dot(a, b));
}

inline void ToCenterAndExtents(float3 min, float3 max, out float3 center, out float3 extents)
{
    center = (min + max) * 0.5f;
    extents = (max - min) * 0.5f;
}

void TransformAABB(float3 minPoint, float3 maxPoint, float4x4 worldMatrix, out float3 newMin, out float3 newMax)
{
    minPoint = mul(float4(minPoint, 1.0f), worldMatrix).xyz;
    maxPoint = mul(float4(maxPoint, 1.0f), worldMatrix).xyz;
        
    const float3 vertices[8] = {
        minPoint,
        float3(minPoint.x, minPoint.y, maxPoint.z),
        float3(minPoint.x, maxPoint.y, minPoint.z),
        float3(minPoint.x, maxPoint.y, maxPoint.z),
        float3(maxPoint.x, minPoint.y, minPoint.z),
        float3(maxPoint.x, minPoint.y, maxPoint.z),
        float3(maxPoint.x, maxPoint.y, minPoint.z),
        maxPoint
    };

    newMin = float3(1e6, 1e6, 1e6);
    newMax = float3(-1e6, -1e6, -1e6);

    [unroll]
    for (int i = 0; i < 8; i++)
    {
        newMin = min(newMin, vertices[i].xyz);
        newMax = max(newMax, vertices[i].xyz);
    }
}

float4 ScreenSpaceToClipSpace(float3 screenSpace, uint2 screenSize)
{
    return float4(screenSpace / float3(screenSize * 2.0f - 1.0f, 1.0f), 1.0f);
}

float4 ClipSpaceToScreenSpace(float4 clipSpace, uint2 screenSize)
{
    return float4((clipSpace.xy / clipSpace.w + 1.0f) * 0.5f * screenSize, clipSpace.z, clipSpace.w);
}

float4 ClipSpaceToViewSpace(float4 clipSpace, float4x4 inverseProjection)
{
    return mul(clipSpace, inverseProjection);
}

float4 MakePlane(float3 planePoint, float3 normal)
{
    return float4(normal, -dot(normal, planePoint));
}

float4 MakePlane(float3 v0, float3 v1, float3 v2)
{
    return MakePlane(v0, cross(v1 - v0, v2 - v0));
}

bool SphereFrustumIntersection(float3 sphereCenter, float sphereRadius, SimpleFrustum frustum)
{
    for (int i = 0; i < 4; i++)
    {
        if (DistanceFromPlane(sphereCenter, frustum.Planes[i].xyz, frustum.Planes[i].w) > sphereRadius)
        {
            return false;
        }
    }

    return true;
}
