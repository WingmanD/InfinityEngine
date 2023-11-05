#pragma once

#include <string>
#include "DirectXMath.h"

class Vector2
{
public:
    static const Vector2 Zero;
    static const Vector2 One;
    static const Vector2 UnitX;
    static const Vector2 UnitY;
    
public:
    explicit Vector2();

    explicit Vector2(float value);

    Vector2(float x, float y);

    Vector2(const Vector2& other);
    Vector2(Vector2&& other) noexcept;

    ~Vector2() = default;

    Vector2& operator=(const Vector2& other);
    Vector2& operator=(Vector2&& other) noexcept;

    Vector2(const DirectX::XMVECTOR& vector);

    void SetX(float value);
    float GetX() const;

    void SetY(float value);
    float GetY() const;
    
    void Normalize();
    void Reflect(const Vector2& normal);
    void Refract(const Vector2& normal, float index);

    static float Dot(const Vector2& lhs, const Vector2& rhs);
    static Vector2 Cross(const Vector2& lhs, const Vector2& rhs);
    static Vector2 Normalize(const Vector2& vector);
    static float Length(const Vector2& vector);
    static float LengthSquared(const Vector2& vector);
    static Vector2 Reflect(const Vector2& vector, const Vector2& normal);
    static Vector2 Refract(const Vector2& vector, const Vector2& normal, float index);

    std::wstring ToString() const;

    operator DirectX::XMVECTOR() const;

    Vector2& operator+=(const Vector2& other);
    Vector2& operator-=(const Vector2& other);
    Vector2& operator*=(const Vector2& other);
    Vector2& operator/=(const Vector2& other);

    friend Vector2 operator+(const Vector2& lhs, const Vector2& rhs);
    friend Vector2 operator-(const Vector2& lhs, const Vector2& rhs);
    friend Vector2 operator*(const Vector2& lhs, const Vector2& rhs);
    friend Vector2 operator/(const Vector2& lhs, const Vector2& rhs);

    friend bool operator==(const Vector2& lhs, const Vector2& rhs);
    friend bool operator!=(const Vector2& lhs, const Vector2& rhs);

    friend bool operator<(const Vector2& lhs, const Vector2& rhs);
    friend bool operator>(const Vector2& lhs, const Vector2& rhs);

private:
    DirectX::XMFLOAT2 _value;
};
