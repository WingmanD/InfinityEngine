#include "Math.h"

Vector3 Math::ToDegrees(const Vector3& radians)
{
    return {ToDegrees(radians.x), ToDegrees(radians.y), ToDegrees(radians.z)};
}

Vector3 Math::ToRadians(const Vector3& degrees)
{
    return {ToRadians(degrees.x), ToRadians(degrees.y), ToRadians(degrees.z)};
}

Vector3 Math::RandomUnitVector()
{
    Vector3 randomVector = {Random(-1.0f, 1.0f), Random(-1.0f, 1.0f), Random(-1.0f, 1.0f)};
    randomVector.Normalize();

    return randomVector;
}

Matrix Math::CreatePerspectiveMatrix(float horizontalFOV, float aspectRatio, float nearClipPlane, float farClipPlane)
{
    Matrix perspective;
    XMStoreFloat4x4(&perspective,DirectX::XMMatrixPerspectiveFovLH(horizontalFOV, aspectRatio, nearClipPlane, farClipPlane));
    return perspective;
}

Vector3 Math::RotateVector(const Vector3& vector, const Vector3& axis, float angle)
{
    return Vector3::TransformNormal(vector, Matrix::CreateFromAxisAngle(axis, angle));
}

Vector3 Math::RotateVector(const Vector3& vector, const Quaternion& rotation)
{
    const Vector3 r(rotation.x, rotation.y, rotation.z);

    return 2.0f * r.Dot(vector) * r + (rotation.w * rotation.w - r.Dot(r)) *
        vector + 2.0f * rotation.w * r.Cross(vector);
}

Vector3 Math::Mirror(const Vector3& vector, const Vector3& normal)
{
    return vector - 2.0f * vector.Dot(normal) * normal;
}

float Math::Distance(const Vector3& point, const Plane& plane)
{
    return point.Dot(plane.Normal()) + plane.D();
}
