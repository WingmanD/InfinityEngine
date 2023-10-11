#include "Vector3.h"
#include <format>

Vector3::Vector3() : Vector3(0.0f)
{
}

Vector3::Vector3(float value)
{
    XMStoreFloat3(&_value, DirectX::XMVectorSet(value, value, value, 0.0f));
}

Vector3::Vector3(float x, float y, float z)
{
    XMStoreFloat3(&_value, DirectX::XMVectorSet(x, y, z, 0.0f));
}

Vector3::Vector3(const Vector3& other)
{
    XMStoreFloat3(&_value, other);
}

Vector3::Vector3(Vector3&& other) noexcept
{
    _value = other._value;
}

Vector3& Vector3::operator=(const Vector3& other)
{
    XMStoreFloat3(&_value, other);
    return *this;
}

Vector3& Vector3::operator=(Vector3&& other) noexcept
{
    _value = other._value;
    return *this;
}

Vector3::Vector3(const DirectX::XMVECTOR& vector)
{
    XMStoreFloat3(&_value, vector);
}

void Vector3::SetX(float value)
{
    DirectX::XMVectorSetX(*this, value);
}

float Vector3::GetX() const
{
    return DirectX::XMVectorGetX(*this);
}

void Vector3::SetY(float value)
{
    DirectX::XMVectorSetY(*this, value);
}

float Vector3::GetY() const
{
    return DirectX::XMVectorGetY(*this);
}

void Vector3::SetZ(float value)
{
    DirectX::XMVectorSetZ(*this, value);
}

float Vector3::GetZ() const
{
    return DirectX::XMVectorGetZ(*this);
}

void Vector3::Normalize()
{
    XMStoreFloat3(&_value, DirectX::XMVector3Normalize(*this));
}

void Vector3::Reflect(const Vector3& normal)
{
    XMStoreFloat3(&_value, DirectX::XMVector3Reflect(*this, normal));
}

void Vector3::Refract(const Vector3& normal, float index)
{
    XMStoreFloat3(&_value, DirectX::XMVector3Refract(*this, normal, index));
}

float Vector3::Dot(const Vector3& lhs, const Vector3& rhs)
{
    return DirectX::XMVectorGetX(DirectX::XMVector3Dot(lhs, rhs));
}

Vector3 Vector3::Cross(const Vector3& lhs, const Vector3& rhs)
{
    return DirectX::XMVector3Cross(lhs, rhs);
}

Vector3 Vector3::Normalize(const Vector3& vector)
{
    return DirectX::XMVector3Normalize(vector);
}

float Vector3::Length(const Vector3& vector)
{
    return DirectX::XMVectorGetX(DirectX::XMVector3Length(vector));
}

float Vector3::LengthSquared(const Vector3& vector)
{
    return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(vector));
}

Vector3 Vector3::Reflect(const Vector3& vector, const Vector3& normal)
{
    return DirectX::XMVector3Reflect(vector, normal);
}

Vector3 Vector3::Refract(const Vector3& vector, const Vector3& normal, float index)
{
    return DirectX::XMVector3Refract(vector, normal, index);
}

std::wstring Vector3::ToString() const
{
    return std::format(L"{{{}, {}, {}}}", GetX(), GetY(), GetZ());
}

Vector3::operator DirectX::XMVECTOR() const
{
    return XMLoadFloat3(&_value);
}

Vector3& Vector3::operator+=(const Vector3& other)
{
    XMStoreFloat3(&_value, DirectX::XMVectorAdd(XMLoadFloat3(&_value), other));
    return *this;
}

Vector3& Vector3::operator-=(const Vector3& other)
{
    XMStoreFloat3(&_value, DirectX::XMVectorSubtract(XMLoadFloat3(&_value), other));
    return *this;
}

Vector3& Vector3::operator*=(const Vector3& other)
{
    XMStoreFloat3(&_value, DirectX::XMVectorMultiply(XMLoadFloat3(&_value), other));
    return *this;
}

Vector3& Vector3::operator/=(const Vector3& other)
{
    XMStoreFloat3(&_value, DirectX::XMVectorDivide(XMLoadFloat3(&_value), other));
    return *this;
}

Vector3 operator+(const Vector3& lhs, const Vector3& rhs)
{
    return DirectX::XMVectorAdd(lhs, rhs);
}
Vector3 operator-(const Vector3& lhs, const Vector3& rhs)
{
    return DirectX::XMVectorSubtract(lhs, rhs);
}

Vector3 operator*(const Vector3& lhs, const Vector3& rhs)
{
    return DirectX::XMVectorMultiply(lhs, rhs);
}

Vector3 operator/(const Vector3& lhs, const Vector3& rhs)
{
    return DirectX::XMVectorDivide(lhs, rhs);
}

bool operator==(const Vector3& lhs, const Vector3& rhs)
{
    return DirectX::XMVectorGetX(DirectX::XMVectorEqual(lhs, rhs));
}

bool operator!=(const Vector3& lhs, const Vector3& rhs)
{
    return !(lhs == rhs);
}

bool operator<(const Vector3& lhs, const Vector3& rhs)
{
    return DirectX::XMVectorGetX(DirectX::XMVectorLess(lhs, rhs));
}

bool operator>(const Vector3& lhs, const Vector3& rhs)
{
    return rhs < lhs;
}
