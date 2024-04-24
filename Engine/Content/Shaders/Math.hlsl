inline float3 VectorPlaneProjection(float3 vec, float3 planeNormal)
{
    return vec - planeNormal * dot(vec, planeNormal);
}

inline float Angle(float3 a, float3 b)
{
    return acos(dot(a, b) / (length(a) * length(b)));
}

inline float AngleFast(float3 a, float3 b)
{
    return acos(dot(a, b));
}