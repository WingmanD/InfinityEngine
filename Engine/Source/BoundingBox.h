#pragma once

#include "Math/MathFwd.h"

class Transform;
class MemoryReader;
class MemoryWriter;

/**
 * Axis-aligned bounding box.
 */
class BoundingBox
{
public:
    BoundingBox() = default;
    BoundingBox(const Vector3& min, const Vector3& max);

    const Vector3& GetMin() const;
    const Vector3& GetMax() const;
    Vector3 GetCenter() const;
    Vector3 GetExtent() const;

    bool Overlap(const BoundingBox& other) const;
    bool Contains(const Vector3& point) const;

    float SweepDistance(const Vector3& direction) const;

    BoundingBox TransformBy(const Transform& transform) const;
    void Move(const Vector3& offset);
    
    BoundingBox Union(const BoundingBox& other) const;

    Vector3 FurthestPointInDirection(const Vector3& direction) const;

    friend MemoryWriter& operator<<(MemoryWriter& writer, const BoundingBox& boundingBox);
    friend MemoryReader& operator>>(MemoryReader& reader, BoundingBox& boundingBox);

private:
    Vector3 _min;
    Vector3 _max;
};

MemoryWriter& operator<<(MemoryWriter& writer, const BoundingBox& boundingBox);
MemoryReader& operator>>(MemoryReader& reader, BoundingBox& boundingBox);
