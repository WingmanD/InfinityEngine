#include "Math.h"

float Line::DistanceToPoint(const Vector3& point) const
{
    const Vector3 line = End - Start;
    return line.Cross(Start - point).Length() / line.Length();
}

Vector3 Line::ClosestPoint(const Vector3& point) const
{
    const Vector3 a = point - Start;
    const Vector3 b = End - Start;
    
    return (a.Dot(b) / b.Length()) * b;
}

Vector2 Math::ToDegrees(const Vector2& radians)
{
    return {ToDegrees(radians.x), ToDegrees(radians.y)};
}

Vector3 Math::ToDegrees(const Vector3& radians)
{
    return {ToDegrees(radians.x), ToDegrees(radians.y), ToDegrees(radians.z)};
}

Vector2 Math::ToRadians(const Vector2& degrees)
{
    return {ToRadians(degrees.x), ToRadians(degrees.y)};
}

Vector3 Math::ToRadians(const Vector3& degrees)
{
    return {ToRadians(degrees.x), ToRadians(degrees.y), ToRadians(degrees.z)};
}

Quaternion Math::MakeQuaternionFromEuler(const Vector3& euler)
{
    return Quaternion::CreateFromYawPitchRoll(
        ToRadians(euler.y),
        ToRadians(euler.x),
        ToRadians(euler.z)
    );
}

Vector3 Math::RandomUnitVector()
{
    Vector3 randomVector = {Random(-1.0f, 1.0f), Random(-1.0f, 1.0f), Random(-1.0f, 1.0f)};
    randomVector.Normalize();

    return randomVector;
}

Vector2 Math::Abs(const Vector2& vector)
{
    return {Abs(vector.x), Abs(vector.y)};
}

Vector3 Math::Abs(const Vector3& vector)
{
    return {Abs(vector.x), Abs(vector.y), Abs(vector.z)};
}

float Math::Min(const Vector2& vector)
{
    return Min(vector.x, vector.y);
}

float Math::Min(const Vector3& vector)
{
    return Min(vector.x, Min(vector.y, vector.z));
}

float Math::Max(const Vector2& vector)
{
    return Max(vector.x, vector.y);
}

float Math::Max(const Vector3& vector)
{
    return Max(vector.x, Max(vector.y, vector.z));
}

Vector2 Math::FloorToInt(const Vector2& value)
{
    return {
        static_cast<float>(FloorToInt(value.x)),
        static_cast<float>(FloorToInt(value.y))
    };
}

Vector3 Math::FloorToInt(const Vector3& value)
{
    return {
        static_cast<float>(FloorToInt(value.x)),
        static_cast<float>(FloorToInt(value.y)),
        static_cast<float>(FloorToInt(value.z))
    };
}

Matrix Math::CreatePerspectiveMatrix(float horizontalFOV, float aspectRatio, float nearClipPlane, float farClipPlane)
{
    Matrix perspective;
    XMStoreFloat4x4(&perspective,DirectX::XMMatrixPerspectiveFovLH(ToRadians(horizontalFOV / 2.0f), aspectRatio, nearClipPlane, farClipPlane));
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

Vector3 Math::TripleVectorProduct(const Vector3& a, const Vector3& b, const Vector3& c)
{
    return a.Cross(b).Cross(c);
}

float Math::Distance(const Vector3& point, const Plane& plane)
{
    return point.Dot(plane.Normal()) + plane.D();
}

Vector3 Math::PlanarProjection(const Vector3& vec, const Vector3& planePoint, const Vector3& planeNormal)
{
    //return planePoint - (planeNormal.Dot(planePoint) - planeNormal.Dot(vec)) * planeNormal;
    return vec - planeNormal * (vec - planePoint).Dot(planeNormal);
}

Vector3 Math::BarycentricCoordinates(const Vector3& point, const Vector3& a, const Vector3& b, const Vector3& c)
{
    const Vector3 v0 = b - a;
    const Vector3 v1 = c - a;
    const Vector3 v2 = point - a;

    const float d00 = v0.Dot(v0);
    const float d01 = v0.Dot(v1);
    const float d11 = v1.Dot(v1);
    const float d20 = v2.Dot(v0);
    const float d21 = v2.Dot(v1);
    const float denominator = d00 * d11 - d01 * d01;

    Vector3 result;
    result.y = (d11 * d20 - d01 * d21) / denominator;
    result.z = (d00 * d21 - d01 * d20) / denominator;
    result.x = 1.0f - result.y - result.z;

    return result;
}

bool Math::IsPointInsideTriangle(const Vector3& point, const Vector3& a, const Vector3& b, const Vector3& c)
{
    return BarycentricInsideTriangle(BarycentricCoordinates(point, a, b, c));
}

bool Math::BarycentricInsideTriangle(const Vector3& barycentric)
{
    return barycentric.x >= 0.0f && barycentric.x <= 1.0f &&
        barycentric.y >= 0.0f && barycentric.y <= 1.0f &&
        barycentric.z >= 0.0f && barycentric.z <= 1.0f &&
        barycentric.x + barycentric.y + barycentric.z <= 1.0f;
}
