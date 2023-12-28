#include "Transform2D.h"
#include "Math.h"

Transform2D Transform2D::FromMatrix(const Matrix& matrix)
{
    Transform2D transform;
    transform._matrix = matrix;
    transform._matrixDirty = false;
    
    transform._position = Vector2(matrix._41, matrix._42);
    transform._rotation = Math::ToDegrees(atan2f(matrix._21, matrix._11));
    transform._scale = Vector2(Vector2(matrix._11, matrix._12).Length(), Vector2(matrix._21, matrix._22).Length());

    return transform;
}

void Transform2D::SetPosition(const Vector2& position)
{
    _position = position;
    _matrixDirty = true;
}

const Vector2& Transform2D::GetPosition() const
{
    return _position;
}

void Transform2D::SetZOffset(float zOffset)
{
    _zOffset = zOffset;
    _matrixDirty = true;
}

float Transform2D::GetZOffset() const
{
    return _zOffset;
}

void Transform2D::SetRotation(float degrees)
{
    _rotation = degrees;
    _matrixDirty = true;
}

float Transform2D::GetRotation() const
{
    return _rotation;
}

void Transform2D::SetScale(const Vector2& scale)
{
    _scale = scale;
    _matrixDirty = true;
}

const Vector2& Transform2D::GetScale() const
{
    return _scale;
}

const Matrix& Transform2D::GetMatrix() const
{
    Transform2D* mutableThis = const_cast<Transform2D*>(this);
    if (_matrixDirty)
    {
        mutableThis->_matrixDirty = false;

        mutableThis->_matrix =
            Matrix::CreateScale(_scale.x, _scale.y, 1.0f) *
            Matrix::CreateRotationZ(Math::ToRadians(_rotation)) *
            Matrix::CreateTranslation(_position.x, _position.y, _zOffset);
    }

    return _matrix;
}

Transform2D::operator const Matrix&() const
{
    return GetMatrix();
}

Transform2D Transform2D::operator*(const Transform2D& other) const
{
    const Matrix matrix = GetMatrix() * other.GetMatrix();
    return FromMatrix(matrix);
}
