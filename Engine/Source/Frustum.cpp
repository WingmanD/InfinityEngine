#include "Frustum.h"
#include "CoreMinimal.h"
#include "Math/Math.h"

Frustum::Frustum(
    const Transform& transform,
    float horizontalFieldOfView,
    float aspectRatio,
    float nearClipPlane,
    float farClipPlane) :
    _transform(transform),
    _horizontalFieldOfView(horizontalFieldOfView),
    _aspectRatio(aspectRatio),
    _nearClipPlane(nearClipPlane),
    _farClipPlane(farClipPlane)
{
    RegeneratePlanes();
}

bool Frustum::Contains(const Vector3& point) const
{
    for (int8 i = 0; i < 6; ++i)
    {
        if (Math::Distance(point, _planes[i]) < 0.0f)
        {
            return false;
        }
    }

    return true;
}

void Frustum::RegeneratePlanes()
{
    const Vector3 forward = _transform.GetForwardVector();
    const Vector3 right = _transform.GetRightVector();
    const Vector3 up = _transform.GetUpVector();

    const Vector3 location = _transform.GetWorldLocation();

    const Vector3 position = location + forward * _nearClipPlane;

    _planes[0] = Plane(position, forward);
    _planes[1] = Plane(location + forward * _farClipPlane, -forward);

    const Vector3 leftNormal = Math::RotateVector(right, up, Math::ToRadians(90.0f - _horizontalFieldOfView / 2.0f));
    _planes[2] = Plane(position, leftNormal);

    const Vector3 rightNormal = Math::Mirror(leftNormal, forward);
    _planes[3] = Plane(position, rightNormal);

    const float verticalFieldOfView = _horizontalFieldOfView / _aspectRatio;

    const Vector3 topNormal = Math::RotateVector(up, right, Math::ToRadians(90.0f - verticalFieldOfView / 2.0f));
    _planes[4] = Plane(position, topNormal);

    const Vector3 bottomNormal = Math::Mirror(topNormal, right);
    _planes[5] = Plane(position, bottomNormal);
}
