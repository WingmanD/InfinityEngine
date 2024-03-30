#pragma once

#include "Component.h"
#include "Math/Transform.h"
#include "CCamera.reflection.h"

REFLECTED()
class CCamera : public Component
{
    GENERATED()

public:
    PROPERTY(Edit, Serialize)
    Transform CameraTransform;

public:
    float GetHorizontalFieldOfView() const;
    void SetHorizontalFieldOfView(float horizontalFieldOfView);

    float GetNearClipPlane() const;
    void SetNearClipPlane(float nearClipPlane);

    float GetFarClipPlane() const;
    void SetFarClipPlane(float farClipPlane);

    float GetAspectRatio() const;
    void SetAspectRatio(float aspectRatio);

    Matrix GetViewMatrix();
    Matrix GetProjectionMatrix();
    Matrix GetViewProjectionMatrix();

private:
    Matrix _viewMatrix;
    Matrix _projectionMatrix;
    Matrix _viewProjectionMatrix;

    bool _isViewMatrixDirty : 1 = true;
    bool _isProjectionMatrixDirty : 1 = true;
    bool _isViewProjectionMatrixDirty : 1 = true;

private:
    PROPERTY(Edit, Serialize, DisplayName = "Horizontal Field of View")
    float _horizontalFieldOfView = 90.0f;

    PROPERTY(Edit, Serialize, DisplayName = "Near Clip Plane")
    float _nearClipPlane = 0.1f;

    PROPERTY(Edit, Serialize, DisplayName = "Far Clip Plane")
    float _farClipPlane = 1000.0f;

    float _aspectRatio = 16.0f / 9.0f;
};
