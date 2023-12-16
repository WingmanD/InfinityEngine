#include "Transform2D.h"
#include "Math.h"

void Transform2D::SetPosition(const Vector2& position)
{
    _position = position;
    _matrixDirty = true;
}

const Vector2& Transform2D::GetPosition() const
{
    return _position;
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
            Matrix::CreateTranslation(_position.x, _position.y, 0.0f);
    }

    return _matrix;
}

Transform2D::operator const Matrix&() const
{
    return GetMatrix();
}
