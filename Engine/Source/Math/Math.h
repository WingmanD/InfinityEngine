#pragma once

#include <numbers>
#include <random>

namespace Math
{
    template <typename T>
    constexpr T ToRadians(T degrees)
    {
        constexpr T mul = static_cast<T>(std::numbers::pi) / T(180.0);
        return degrees * mul;
    }

    template <typename T>
    constexpr T ToDegrees(T radians)
    {
        constexpr T mul = T(180.0) / static_cast<T>(std::numbers::pi);
        return radians * mul;
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
