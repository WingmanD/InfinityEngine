#include "BoundingBox2D.h"

BoundingBox2D::BoundingBox2D(const Vector2& position, const Vector2& size)
{
    const Vector2 halfSize = size * 0.5f;
    
    _min = position - halfSize;
    _max = position + halfSize;
}

bool BoundingBox2D::Intersects(const BoundingBox2D& other) const
{
    return (_min.x <= other._max.x && _max.x >= other._min.x) &&
        (_min.y <= other._max.y && _max.y >= _min.y);
}

bool BoundingBox2D::Contains(const Vector2& point) const
{
    return (point.x >= _min.x && point.x <= _max.x) &&
        (point.y >= _min.y && point.y <= _max.y);
}

void BoundingBox2D::Move(const Vector2& offset)
{
    _min += offset;
    _max += offset;
}

void BoundingBox2D::MoveTo(const Vector2& position)
{
    const Vector2 size = _max - _min;
    const Vector2 halfSize = size * 0.5f;
    
    _max = position + halfSize;
    _min = position - halfSize;
}

const Vector2& BoundingBox2D::GetMin() const
{
    return _min;
}

const Vector2& BoundingBox2D::GetMax() const
{
    return _max;
}
