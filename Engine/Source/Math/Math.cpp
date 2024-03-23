#include "Math.h"

Vector3 Math::ToDegrees(const Vector3& radians)
{
    return {ToDegrees(radians.x), ToDegrees(radians.y), ToDegrees(radians.z)};
}

Vector3 Math::ToRadians(const Vector3& degrees)
{
    return {ToRadians(degrees.x), ToRadians(degrees.y), ToRadians(degrees.z)};
}
