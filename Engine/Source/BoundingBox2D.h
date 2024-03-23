#pragma once

#include "Core.h"

class BoundingBox2D
{
public:
    BoundingBox2D() = default;
    BoundingBox2D(const Vector2& position, const Vector2& size);

    bool Intersects(const BoundingBox2D& other) const;
    bool Contains(const Vector2& point) const;

   static std::optional<BoundingBox2D> Intersection(const BoundingBox2D& lhs, const BoundingBox2D& rhs);

    void Move(const Vector2& offset);
    void MoveTo(const Vector2& position);

    const Vector2& GetMin() const;
    const Vector2& GetMax() const;

private:
    Vector2 _min;
    Vector2 _max;
};
