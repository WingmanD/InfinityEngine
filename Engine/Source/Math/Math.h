#pragma once

#include <numbers>

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
}
