#include "BoundingBox.h"
#include "MemoryReader.h"
#include "MemoryWriter.h"

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
