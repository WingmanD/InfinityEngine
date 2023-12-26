#pragma once

#include "Core.h"

class Transform2D
{
public:
    Transform2D() = default;

    // todo copy constructor

    static Transform2D FromMatrix(const Matrix& matrix);

    void SetPosition(const Vector2& position);
    const Vector2& GetPosition() const;

    void SetRotation(float degrees);
    float GetRotation() const;

    void SetScale(const Vector2& scale);
    const Vector2& GetScale() const;
    const Matrix& GetMatrix() const;
    
    operator const Matrix&() const;

    Transform2D operator*(const Transform2D& other) const;
    
private:
    Vector2 _position = Vector2::Zero;
    float _rotation = 0.0f;
    Vector2 _scale = Vector2::One;
    
    Matrix _matrix = Matrix::Identity;
    bool _matrixDirty = true;
};
