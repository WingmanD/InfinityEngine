#pragma once

#include "MathFwd.h"
#include <numbers>
#include <random>

struct Line
{
public:
    Vector3 Start;
    Vector3 End;

public:
    float DistanceToPoint(const Vector3& point) const;
    Vector3 ClosestPoint(const Vector3& point) const;
};

namespace Math
{
    constexpr auto Pi = std::numbers::pi_v<float>;
    
    constexpr auto Square(auto value)
    {
        return value * value;
    }

    template <typename T>
    constexpr T ToRadians(T degrees)
    {
        constexpr T mul = std::numbers::pi_v<T> / T(180.0);
        return degrees * mul;
    }

    template <typename T>
    constexpr T ToDegrees(T radians)
    {
        constexpr T mul = T(180.0) / std::numbers::pi_v<T>;
        return radians * mul;
    }

    template <typename T>
    constexpr T RoundToNearest(T value, T multiple)
    {
        return static_cast<T>((value + multiple / 2.0) / (multiple * multiple));
    }

    template <typename T> requires std::is_integral_v<T>
    constexpr T NextPowerOfTwo(T value)
    {
        --value;
        for (size_t i = 1; i < sizeof(T) * 8; i *= 2)
        {
            value |= value >> i;
        }

        return ++value;
    }

    constexpr bool IsPowerOfTwo(uint32 value)
    {
        return value && !(value & (value - 1));
    }

    template <uint32 N> requires (IsPowerOfTwo(N))
    constexpr uint32 NextCircularIndex(uint32 currentIndex)
    {
        return (N - 1) & (currentIndex + 1);
    }

    Vector2 ToDegrees(const Vector2& radians);
    Vector3 ToDegrees(const Vector3& radians);
    
    Vector2 ToRadians(const Vector2& degrees);
    Vector3 ToRadians(const Vector3& degrees);

    Quaternion MakeQuaternionFromEuler(const Vector3& euler);

    template <typename T> requires std::is_arithmetic_v<T>
    T Random(T min, T max)
    {
        std::random_device randomDevice;
        std::mt19937 generator(randomDevice());
        std::uniform_real_distribution<T> distribution(min, max);

        return distribution(generator);
    }

    template <typename T> requires std::is_arithmetic_v<T>
    T Random()
    {
        return Random(static_cast<T>(0), static_cast<T>(1));
    }

    Vector3 RandomUnitVector();

    Vector2 Abs(const Vector2& vector);
    Vector3 Abs(const Vector3& vector);
    
    float Min(const Vector2& vector);
    float Min(const Vector3& vector);
    
    float Max(const Vector2& vector);
    float Max(const Vector3& vector);

    auto Min(auto value1, auto value2)
    {
        return value1 < value2 ? value1 : value2;
    }

    auto Max(auto value1, auto value2)
    {
        return value1 > value2 ? value1 : value2;
    }

    constexpr float Clamp(float value, float min, float max)
    {
        return value < min ? min : value > max ? max : value;
    }

    constexpr auto Abs(auto value)
    {
        return value < 0 ? -value : value;
    }

    constexpr auto Ceil(auto value)
    {
        return static_cast<uint64>(value) + value / static_cast<uint64>(value) - 1;
    }

    constexpr int64 FloorToInt(auto value)
    {
        const int64 integer = static_cast<int64>(value);
        if (value < 0 && value != integer)
        {
            return integer - 1;
        }

        return integer;
    }

    Vector2 FloorToInt(const Vector2& value);
    Vector3 FloorToInt(const Vector3& value);

    constexpr int32 Sign(auto value)
    {
        return value < 0 ? -1 : value > 0 ? 1 : 0;
    }

    Matrix CreatePerspectiveMatrix(float horizontalFOV, float aspectRatio, float nearClipPlane, float farClipPlane);

    Vector3 RotateVector(const Vector3& vector, const Vector3& axis, float angle);
    Vector3 RotateVector(const Vector3& vector, const Quaternion& rotation);

    Vector3 Mirror(const Vector3& vector, const Vector3& normal);

    Vector3 TripleVectorProduct(const Vector3& a, const Vector3& b, const Vector3& c);

    /**
     * Calculates the distance between a point and a plane.
     * Returns a negative value when point is underneath the plane.
     */
    float Distance(const Vector3& point, const Plane& plane);

    Vector3 PlanarProjection(const Vector3& vec, const Vector3& planePoint, const Vector3& planeNormal);

    Vector3 BarycentricCoordinates(const Vector3& point, const Vector3& a, const Vector3& b, const Vector3& c);
    bool IsPointInsideTriangle(const Vector3& point, const Vector3& a, const Vector3& b, const Vector3& c);
    bool BarycentricInsideTriangle(const Vector3& barycentric);
}
