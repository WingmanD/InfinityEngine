#pragma once

#include "Math/MathFwd.h"

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

    friend MemoryWriter& operator<<(MemoryWriter& writer, const BoundingBox& boundingBox);
    friend MemoryReader& operator>>(MemoryReader& reader, BoundingBox& boundingBox);
    
private:
    Vector3 _min;
    Vector3 _max;
};

MemoryWriter& operator<<(MemoryWriter& writer, const BoundingBox& boundingBox);
MemoryReader& operator>>(MemoryReader& reader, BoundingBox& boundingBox);
