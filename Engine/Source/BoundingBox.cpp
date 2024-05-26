#include "BoundingBox.h"
#include "MemoryReader.h"
#include "MemoryWriter.h"
#include "Math/Math.h"
#include "Math/Transform.h"

BoundingBox::BoundingBox(const Vector3& min, const Vector3& max) : _min(min), _max(max)
{
}

const Vector3& BoundingBox::GetMin() const
{
    return _min;
}

const Vector3& BoundingBox::GetMax() const
{
    return _max;
}

Vector3 BoundingBox::GetCenter() const
{
    return (_min + _max) * 0.5f;
}

Vector3 BoundingBox::GetExtent() const
{
    return (_max - _min) * 0.5f;
}

bool BoundingBox::Overlap(const BoundingBox& other) const
{
    return _min.x <= other._max.x && _max.x >= other._min.x &&
        _min.y <= other._max.y && _max.y >= other._min.y &&
        _min.z <= other._max.z && _max.z >= other._min.z;
}

bool BoundingBox::Contains(const Vector3& point) const
{
    return point.x >= _min.x && point.x <= _max.x &&
        point.y >= _min.y && point.y <= _max.y &&
        point.z >= _min.z && point.z <= _max.z;
}

float BoundingBox::SweepDistance(const Vector3& direction) const
{
    return Math::Min(GetExtent() / Math::Abs(direction)) * 2.0f;
}

BoundingBox BoundingBox::TransformBy(const Transform& transform) const
{
    const Vector3 minPoint = transform * _min;
    const Vector3 maxPoint = transform * _max;

    const Vector3 vertices[8] = {
        minPoint,
        Vector3(minPoint.x, minPoint.y, maxPoint.z),
        Vector3(minPoint.x, maxPoint.y, minPoint.z),
        Vector3(minPoint.x, maxPoint.y, maxPoint.z),
        Vector3(maxPoint.x, minPoint.y, minPoint.z),
        Vector3(maxPoint.x, minPoint.y, maxPoint.z),
        Vector3(maxPoint.x, maxPoint.y, minPoint.z),
        maxPoint
    };

    Vector3 newMin = Vector3(1e6, 1e6, 1e6);
    Vector3 newMax = Vector3(-1e6, -1e6, -1e6);

    for (int32 i = 0; i < 8; i++)
    {
        newMin = Vector3::Min(newMin, vertices[i]);
        newMax = Vector3::Max(newMax, vertices[i]);
    }

    return {newMin, newMax};
}

void BoundingBox::Move(const Vector3& offset)
{
    _min += offset;
    _max += offset;
}

BoundingBox BoundingBox::Union(const BoundingBox& other) const
{
    Vector3 newMin = Vector3::Min(_min, other._min);
    Vector3 newMax = Vector3::Max(_max, other._max);
    return {newMin, newMax};
}

Vector3 BoundingBox::FurthestPointInDirection(const Vector3& direction) const
{
    return {
        direction.x > 0.0f ? _max.x : _min.x,
        direction.y > 0.0f ? _max.y : _min.y,
        direction.z > 0.0f ? _max.z : _min.z
    };
}

MemoryWriter& operator<<(MemoryWriter& writer, const BoundingBox& boundingBox)
{
    writer << boundingBox.GetMin();
    writer << boundingBox.GetMax();

    return writer;
}

MemoryReader& operator>>(MemoryReader& reader, BoundingBox& boundingBox)
{
    reader >> boundingBox._min;
    reader >> boundingBox._max;

    return reader;
}
