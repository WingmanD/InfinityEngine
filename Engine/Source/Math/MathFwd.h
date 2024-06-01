#pragma once

#include "SimpleMath.h"
#include <format>

typedef DirectX::SimpleMath::Vector2 Vector2;
typedef DirectX::SimpleMath::Vector3 Vector3;
typedef DirectX::SimpleMath::Vector4 Vector4;
typedef DirectX::SimpleMath::Matrix Matrix;
typedef DirectX::SimpleMath::Quaternion Quaternion;
typedef DirectX::SimpleMath::Plane Plane;
typedef DirectX::SimpleMath::Ray Ray;
typedef DirectX::SimpleMath::Color Color;

template <>
struct std::formatter<Vector2, wchar_t>
{
    constexpr auto parse(std::wformat_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const Vector2& vector, std::wformat_context& ctx) const
    {
        return std::format_to(ctx.out(), L"X: {}, Y: {}", vector.x, vector.y);
    }
};

template <>
struct std::formatter<Vector3, wchar_t>
{
    constexpr auto parse(std::wformat_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const Vector3& vector, std::wformat_context& ctx) const
    {
        return std::format_to(ctx.out(), L"X: {}, Y: {}, Z: {}", vector.x, vector.y, vector.z);
    }
};

template <>
struct std::formatter<Vector4, wchar_t>
{
    constexpr auto parse(std::wformat_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const Vector4& vector, std::wformat_context& ctx) const
    {
        return std::format_to(ctx.out(), L"X: {}, Y: {}, Z: {}, W: {}", vector.x, vector.y, vector.z, vector.w);
    }
};

template <>
struct std::formatter<Quaternion, wchar_t>
{
    constexpr auto parse(std::wformat_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const Quaternion& quat, std::wformat_context& ctx) const
    {
        return std::format_to(ctx.out(), L"X: {}, Y: {}, Z: {}, W: {}", quat.x, quat.y, quat.z, quat.w);
    }
};
