#pragma once

#include "MathFwd.h"
#include <numbers>
#include <random>

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
        return static_cast<T>((value + multiple / 2) / (multiple * multiple));
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

    Vector3 ToDegrees(const Vector3& radians);
    Vector3 ToRadians(const Vector3& degrees);

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

    Vector3 Abs(const Vector3& vector);
    float Min(const Vector3& vector);

    auto Min(auto value1, auto value2)
    {
        return value1 < value2 ? value1 : value2;
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

    Matrix CreatePerspectiveMatrix(float horizontalFOV, float aspectRatio, float nearClipPlane, float farClipPlane);

    Vector3 RotateVector(const Vector3& vector, const Vector3& axis, float angle);
    Vector3 RotateVector(const Vector3& vector, const Quaternion& rotation);

    Vector3 Mirror(const Vector3& vector, const Vector3& normal);

    /**
     * Calculates the distance between a point and a plane.
     * Returns a negative value when point is underneath the plane.
     */
    float Distance(const Vector3& point, const Plane& plane);
}
