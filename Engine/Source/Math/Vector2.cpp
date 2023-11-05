#include "Vector2.h"
#include <format>

const Vector2 Vector2::Zero = Vector2(0.0f);
const Vector2 Vector2::One = Vector2(1.0f);
const Vector2 Vector2::UnitX = Vector2(1.0f, 0.0f);
const Vector2 Vector2::UnitY = Vector2(0.0f, 1.0f);

Vector2::Vector2() : Vector2(0.0f)
{
}

Vector2::Vector2(float value)
{
    XMStoreFloat2(&_value, DirectX::XMVectorSet(value, value, 0.0f, 0.0f));
}

Vector2::Vector2(float x, float y)
{
    XMStoreFloat2(&_value, DirectX::XMVectorSet(x, y, 0.0f, 0.0f));
}

Vector2::Vector2(const Vector2& other)
{
    XMStoreFloat2(&_value, other);
}

Vector2::Vector2(Vector2&& other) noexcept
{
    _value = other._value;
}

Vector2& Vector2::operator=(const Vector2& other)
{
    XMStoreFloat2(&_value, other);
    return *this;
}

Vector2& Vector2::operator=(Vector2&& other) noexcept
{
    _value = other._value;
    return *this;
}

Vector2::Vector2(const DirectX::XMVECTOR& vector)
{
    XMStoreFloat2(&_value, vector);
}

void Vector2::SetX(float value)
{
    DirectX::XMVectorSetX(*this, value);
}

float Vector2::GetX() const
{
    return DirectX::XMVectorGetX(*this);
}

void Vector2::SetY(float value)
{
    DirectX::XMVectorSetY(*this, value);
}

float Vector2::GetY() const
{
    return DirectX::XMVectorGetY(*this);
}

void Vector2::Normalize()
{
    XMStoreFloat2(&_value, DirectX::XMVector2Normalize(*this));
}

void Vector2::Reflect(const Vector2& normal)
{
    XMStoreFloat2(&_value, DirectX::XMVector2Reflect(*this, normal));
}

void Vector2::Refract(const Vector2& normal, float index)
{
    XMStoreFloat2(&_value, DirectX::XMVector2Refract(*this, normal, index));
}

float Vector2::Dot(const Vector2& lhs, const Vector2& rhs)
{
    return DirectX::XMVectorGetX(DirectX::XMVector2Dot(lhs, rhs));
}

Vector2 Vector2::Cross(const Vector2& lhs, const Vector2& rhs)
{
    return DirectX::XMVector2Cross(lhs, rhs);
}

Vector2 Vector2::Normalize(const Vector2& vector)
{
    return DirectX::XMVector2Normalize(vector);
}

float Vector2::Length(const Vector2& vector)
{
    return DirectX::XMVectorGetX(DirectX::XMVector2Length(vector));
}

float Vector2::LengthSquared(const Vector2& vector)
{
    return DirectX::XMVectorGetX(DirectX::XMVector2LengthSq(vector));
}

Vector2 Vector2::Reflect(const Vector2& vector, const Vector2& normal)
{
    return DirectX::XMVector2Reflect(vector, normal);
}

Vector2 Vector2::Refract(const Vector2& vector, const Vector2& normal, float index)
{
    return DirectX::XMVector2Refract(vector, normal, index);
}

std::wstring Vector2::ToString() const
{
    return std::format(L"{{{}, {}}}", GetX(), GetY());
}

Vector2::operator DirectX::XMVECTOR() const
{
    return XMLoadFloat2(&_value);
}

Vector2& Vector2::operator+=(const Vector2& other)
{
    XMStoreFloat2(&_value, DirectX::XMVectorAdd(XMLoadFloat2(&_value), other));
    return *this;
}

Vector2& Vector2::operator-=(const Vector2& other)
{
    XMStoreFloat2(&_value, DirectX::XMVectorSubtract(XMLoadFloat2(&_value), other));
    return *this;
}

Vector2& Vector2::operator*=(const Vector2& other)
{
    XMStoreFloat2(&_value, DirectX::XMVectorMultiply(XMLoadFloat2(&_value), other));
    return *this;
}

Vector2& Vector2::operator/=(const Vector2& other)
{
    XMStoreFloat2(&_value, DirectX::XMVectorDivide(XMLoadFloat2(&_value), other));
    return *this;
}

Vector2 operator+(const Vector2& lhs, const Vector2& rhs)
{
    return DirectX::XMVectorAdd(lhs, rhs);
}
Vector2 operator-(const Vector2& lhs, const Vector2& rhs)
{
    return DirectX::XMVectorSubtract(lhs, rhs);
}

Vector2 operator*(const Vector2& lhs, const Vector2& rhs)
{
    return DirectX::XMVectorMultiply(lhs, rhs);
}

Vector2 operator/(const Vector2& lhs, const Vector2& rhs)
{
    return DirectX::XMVectorDivide(lhs, rhs);
}

bool operator==(const Vector2& lhs, const Vector2& rhs)
{
    return DirectX::XMVectorGetX(DirectX::XMVectorEqual(lhs, rhs));
}

bool operator!=(const Vector2& lhs, const Vector2& rhs)
{
    return !(lhs == rhs);
}

bool operator<(const Vector2& lhs, const Vector2& rhs)
{
    return DirectX::XMVectorGetX(DirectX::XMVectorLess(lhs, rhs));
}

bool operator>(const Vector2& lhs, const Vector2& rhs)
{
    return rhs < lhs;
}
