#pragma once

#include "MathFwd.h"

class MemoryWriter;
class MemoryReader;

class Transform
{
public:
    explicit Transform() = default;
    explicit Transform(const Vector3& location, const Quaternion& rotation, const Vector3& scale);

    explicit Transform(const Transform& other);
    explicit Transform(Transform&& other) noexcept;

    Transform& operator=(const Transform& other);
    Transform& operator=(Transform&& other) noexcept;

    ~Transform() = default;

    void SetParent(Transform* parent);
    Transform* GetParent() const;

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

    friend MemoryWriter& operator<<(MemoryWriter& writer, const Transform& transform);
    friend MemoryReader& operator>>(MemoryReader& reader, Transform& transform);
    
private:
    Transform* _parent = nullptr;

    Vector3 _location;
    Quaternion _rotation;
    Vector3 _scale = Vector3::One;

    Matrix _worldMatrix;
    bool _isWorldMatrixDirty = true;
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
