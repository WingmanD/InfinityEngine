#pragma once

#include <string>
#include "DirectXMath.h"

class Vector3
{
public:
    explicit Vector3();

    explicit Vector3(float value);

    Vector3(float x, float y, float z);

    Vector3(const Vector3& other);
    Vector3(Vector3&& other) noexcept;

    ~Vector3() = default;

    Vector3& operator=(const Vector3& other);
    Vector3& operator=(Vector3&& other) noexcept;

    Vector3(const DirectX::XMVECTOR& vector);

    void SetX(float value);
    float GetX() const;

    void SetY(float value);
    float GetY() const;

    void SetZ(float value);
    float GetZ() const;

    void Normalize();
    void Reflect(const Vector3& normal);
    void Refract(const Vector3& normal, float index);
    
    static float Dot(const Vector3& lhs, const Vector3& rhs);
    static Vector3 Cross(const Vector3& lhs, const Vector3& rhs);
    static Vector3 Normalize(const Vector3& vector);
    static float Length(const Vector3& vector);
    static float LengthSquared(const Vector3& vector);
    static Vector3 Reflect(const Vector3& vector, const Vector3& normal);
    static Vector3 Refract(const Vector3& vector, const Vector3& normal, float index);

    std::wstring ToString() const;

    operator DirectX::XMVECTOR() const;

    Vector3& operator+=(const Vector3& other);
    Vector3& operator-=(const Vector3& other);
    Vector3& operator*=(const Vector3& other);
    Vector3& operator/=(const Vector3& other);

    friend Vector3 operator+(const Vector3& lhs, const Vector3& rhs);
    friend Vector3 operator-(const Vector3& lhs, const Vector3& rhs);
    friend Vector3 operator*(const Vector3& lhs, const Vector3& rhs);
    friend Vector3 operator/(const Vector3& lhs, const Vector3& rhs);

    friend bool operator==(const Vector3& lhs, const Vector3& rhs);
    friend bool operator!=(const Vector3& lhs, const Vector3& rhs);

    friend bool operator<(const Vector3& lhs, const Vector3& rhs);
    friend bool operator>(const Vector3& lhs, const Vector3& rhs);

private:
    DirectX::XMFLOAT3 _value;
};
