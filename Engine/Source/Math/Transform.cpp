#include "Transform.h"
#include "Math.h"
#include "MemoryReader.h"
#include "MemoryWriter.h"

Transform::Transform(const Vector3& location, const Quaternion& rotation, const Vector3& scale) :
    _location(location),
    _rotation(rotation),
    _scale(scale)
{
}

Transform::Transform(const Transform& other)
{
    _parent = other._parent;

    _location = other._location;
    _rotation = other._rotation;
    _scale = other._scale;

    _worldMatrix = other._worldMatrix;
}

Transform::Transform(Transform&& other) noexcept
{
    _parent = other._parent;

    _location = std::move(other._location);
    _rotation = std::move(other._rotation);
    _scale = std::move(other._scale);

    _worldMatrix = std::move(other._worldMatrix);
}

Transform& Transform::operator=(const Transform& other)
{
    if (this != &other)
    {
        _parent = other._parent;

        _location = other._location;
        _rotation = other._rotation;
        _scale = other._scale;

        _worldMatrix = other._worldMatrix;
    }

    return *this;
}

Transform& Transform::operator=(Transform&& other) noexcept
{
    if (this != &other)
    {
        _parent = other._parent;

        _location = std::move(other._location);
        _rotation = std::move(other._rotation);
        _scale = std::move(other._scale);

        _worldMatrix = std::move(other._worldMatrix);
    }

    return *this;
}

void Transform::SetParent(Transform* parent)
{
    _parent = parent;

    MarkDirty();
}

Transform* Transform::GetParent() const
{
    return _parent;
}

void Transform::SetRelativeLocation(const Vector3& location)
{
    _location = location;

    MarkDirty();
}

const Vector3& Transform::GetRelativeLocation() const
{
    return _location;
}

void Transform::SetWorldLocation(const Vector3& location)
{
    if (_parent != nullptr)
    {
        Vector3::Transform(location, _parent->GetWorldMatrix().Invert(), _location);
    }
    else
    {
        _location = location;
    }

    MarkDirty();
}

Vector3 Transform::GetWorldLocation() const
{
    if (_parent != nullptr)
    {
        Vector3 location;
        Vector3::Transform(_location, _parent->GetWorldMatrix(), location);

        return location;
    }

    return _location;
}

void Transform::SetRelativeRotation(const Quaternion& rotation)
{
    _rotation = rotation;
    _rotation.Normalize();

    MarkDirty();
}

const Quaternion& Transform::GetRelativeRotation() const
{
    return _rotation;
}

void Transform::SetWorldRotation(const Quaternion& rotation)
{
    Quaternion rotationNormalized = rotation;
    rotationNormalized.Normalize();

    if (_parent != nullptr)
    {
        Quaternion parentRotationConjugate = Quaternion::CreateFromRotationMatrix(_parent->GetWorldMatrix());
        parentRotationConjugate.Conjugate();

        _rotation = rotationNormalized * parentRotationConjugate;
    }
    else
    {
        _rotation = rotationNormalized;
    }

    MarkDirty();
}

Quaternion Transform::GetWorldRotation() const
{
    if (_parent != nullptr)
    {
        return _parent->GetWorldRotation() * _rotation;
    }

    return _rotation;
}

void Transform::SetRelativeRotation(const Vector3& axis, float angle)
{
    _rotation = Quaternion::CreateFromAxisAngle(axis, angle);

    MarkDirty();
}

void Transform::SetWorldRotation(const Vector3& axis, float angle)
{
    if (_parent != nullptr)
    {
        Quaternion parentRotationConjugate = Quaternion::CreateFromRotationMatrix(_parent->GetWorldMatrix());
        parentRotationConjugate.Conjugate();

        _rotation = Quaternion::CreateFromAxisAngle(axis, angle) * parentRotationConjugate;
    }
    else
    {
        _rotation = Quaternion::CreateFromAxisAngle(axis, angle);
    }

    MarkDirty();
}

void Transform::SetRelativeRotation(const Vector3& eulerAngles)
{
    _rotation = Quaternion::CreateFromYawPitchRoll(
        Math::ToRadians(eulerAngles.y),
        Math::ToRadians(eulerAngles.x),
        Math::ToRadians(eulerAngles.z)
    );

    MarkDirty();
}

Vector3 Transform::GetRelativeRotationEuler() const
{
    return Math::ToDegrees(_rotation.ToEuler());
}

void Transform::SetWorldRotation(const Vector3& eulerAngles)
{
    SetRelativeRotation(eulerAngles);

    if (_parent != nullptr)
    {
        Quaternion parentRotationConjugate = Quaternion::CreateFromRotationMatrix(_parent->GetWorldMatrix());
        parentRotationConjugate.Conjugate();

        _rotation *= parentRotationConjugate;
    }
}

Vector3 Transform::GetWorldRotationEuler() const
{
    return Math::ToDegrees(GetWorldRotation().ToEuler());
}

void Transform::SetRelativeScale(const Vector3& scale)
{
    _scale = scale;
}

const Vector3& Transform::GetRelativeScale() const
{
    return _scale;
}

void Transform::SetWorldScale(const Vector3& scale)
{
    if (_parent != nullptr)
    {
        _scale = scale / _parent->GetWorldScale();
    }
    else
    {
        _scale = scale;
    }

    MarkDirty();
}

Vector3 Transform::GetWorldScale() const
{
    if (_parent != nullptr)
    {
        return _scale * _parent->GetWorldScale();
    }

    return _scale;
}

Vector3 Transform::GetForwardVector() const
{
    return Vector3::Transform(Vector3::UnitX, GetWorldRotation());
}

Vector3 Transform::GetRightVector() const
{
    return Vector3::Transform(Vector3::UnitY, GetWorldRotation());
}

Vector3 Transform::GetUpVector() const
{
    return Vector3::Transform(Vector3::UnitZ, GetWorldRotation());
}

const Matrix& Transform::GetWorldMatrix() const
{
    if (!IsWorldMatrixDirty())
    {
        return _worldMatrix;
    }

    Transform* mutableThis = const_cast<Transform*>(this);

    mutableThis->_worldMatrix =
        Matrix::CreateScale(_scale) *
        Matrix::CreateFromQuaternion(_rotation) *
        Matrix::CreateTranslation(_location);

    if (_parent != nullptr)
    {
        mutableThis->_worldMatrix *= _parent->GetWorldMatrix();
        mutableThis->_parentVersion = _parent->_version;
    }

    mutableThis->_isWorldMatrixDirty = false;

    return _worldMatrix;
}

bool Transform::IsWorldMatrixDirty() const
{
    if (_parent != nullptr)
    {
        return _isWorldMatrixDirty || _parent->IsWorldMatrixDirty() || _parentVersion != _parent->_version;
    }

    return _isWorldMatrixDirty;
}

Vector3 Transform::operator*(const Vector3& vector) const
{
    return Vector3::Transform(vector, GetWorldMatrix());
}

Vector4 Transform::operator*(const Vector4& vector) const
{
    return Vector4::Transform(vector, GetWorldMatrix());
}

void Transform::MarkDirty()
{
    _isWorldMatrixDirty = true;
    ++_version;
}

MemoryWriter& operator<<(MemoryWriter& writer, const Transform& transform)
{
    writer << transform._location;
    writer << transform._rotation;
    writer << transform._scale;

    return writer;
}

MemoryReader& operator>>(MemoryReader& reader, Transform& transform)
{
    reader >> transform._location;
    reader >> transform._rotation;
    reader >> transform._scale;

    return reader;
}
