#pragma once

#include "Math/MathFwd.h"
#include "Math/Transform.h"

class Frustum
{
public:
    Frustum(const Transform& transform, float horizontalFieldOfView, float aspectRatio, float nearClipPlane, float farClipPlane);

    bool Contains(const Vector3& point) const;
    //bool Intersects(const BoundingBox& box) const;

private:
    Transform _transform;

    float _horizontalFieldOfView;
    float _aspectRatio;
    float _nearClipPlane;
    float _farClipPlane;

    /**
     * Near, far, left, right, top, bottom.
     */
    Plane _planes[6];

private:
    void RegeneratePlanes();
};
