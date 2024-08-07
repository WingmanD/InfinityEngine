﻿#pragma once

#include "CoreMinimal.h"
#include "MathFwd.h"

class MemoryWriter;
class MemoryReader;
class BoundingBox;

class Transform
{
public:
    explicit Transform() = default;
    explicit Transform(const Vector3& location, const Quaternion& rotation, const Vector3& scale);

    Transform(const Transform& other);
    Transform(Transform&& other) noexcept;

    Transform& operator=(const Transform& other);
    Transform& operator=(Transform&& other) noexcept;

    ~Transform() = default;

    void SetParent(const Transform* parent);
    const Transform* GetParent() const;

    void SetRelativeLocation(const Vector3& location);
    const Vector3& GetRelativeLocation() const;

    void SetWorldLocation(const Vector3& location);
    Vector3 GetWorldLocation() const;

    void SetRelativeRotation(const Quaternion& rotation);
    const Quaternion& GetRelativeRotation() const;

    void SetWorldRotation(const Quaternion& rotation);
    Quaternion GetWorldRotation() const;

    void SetRelativeRotation(const Vector3& axis, float angle);
    void SetWorldRotation(const Vector3& axis, float angle);

    void SetRelativeRotation(const Vector3& eulerAngles);
    Vector3 GetRelativeRotationEuler() const;
    
    void SetWorldRotation(const Vector3& eulerAngles);
    Vector3 GetWorldRotationEuler() const;

    void SetRelativeScale(const Vector3& scale);
    const Vector3& GetRelativeScale() const;

    void SetWorldScale(const Vector3& scale);
    Vector3 GetWorldScale() const;

    Vector3 GetForwardVector() const;
    Vector3 GetRightVector() const;
    Vector3 GetUpVector() const;
    
    const Matrix& GetWorldMatrix() const;
    bool IsWorldMatrixDirty() const;

    Vector3 TransformDirection(const Vector3& direction) const;

    Vector3 operator*(const Vector3& vector) const;
    Vector4 operator*(const Vector4& vector) const;

    Matrix operator*(const Matrix& matrix) const;

    friend MemoryWriter& operator<<(MemoryWriter& writer, const Transform& transform);
    friend MemoryReader& operator>>(MemoryReader& reader, Transform& transform);
    
private:
    const Transform* _parent = nullptr;

    Vector3 _location;
    Quaternion _rotation;
    Vector3 _scale = Vector3::One;

    Matrix _worldMatrix;

    uint32 _version = 0;
    uint32 _parentVersion = 0;
    bool _isWorldMatrixDirty = true;

private:
    void MarkDirty();
};

MemoryWriter& operator<<(MemoryWriter& writer, const Transform& transform);
MemoryReader& operator>>(MemoryReader& reader, Transform& transform);

template <>
struct std::formatter<Transform, wchar_t>
{
    constexpr auto parse(std::wformat_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const Transform& transform, std::wformat_context& ctx) const
    {
        return std::format_to(ctx.out(), L"Location: {} Rotation: {} Scale: {}",
                              transform.GetWorldLocation(),
                              transform.GetWorldRotationEuler(),
                              transform.GetWorldScale());
    }
};
