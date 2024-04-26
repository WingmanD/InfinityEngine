inline float3 VectorPlaneProjection(float3 vec, float3 planeNormal)
{
    return vec - planeNormal * dot(vec, planeNormal);
}

inline float DistanceFromPlane(float3 plane, float3 planeNormal, float3 planePoint)
{
    return abs(dot(planeNormal, plane) - dot(planeNormal, planePoint)) / length(planeNormal);
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
        const float4 transformedVertex = mul(float4(vertices[i], 1.0f), worldMatrix);
        newMin = min(newMin, transformedVertex.xyz);
        newMax = max(newMax, transformedVertex.xyz);
    }
}
