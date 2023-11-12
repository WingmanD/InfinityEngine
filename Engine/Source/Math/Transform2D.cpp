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

const Matrix& Transform2D::GetMatrix()
{
    if (_matrixDirty)
    {
        _matrixDirty = false;

        _matrix = Matrix::CreateRotationZ(Math::ToRadians(_rotation)) *
            Matrix::CreateScale(_scale.x * 2.0f, _scale.y * 2.0f, 1.0f) *
            Matrix::CreateTranslation(_position.x * 2.0f, _position.y * 2.0f, 0.0f);
    }

    return _matrix;
}

Transform2D::operator const Matrix&()
{
    return GetMatrix();
}
