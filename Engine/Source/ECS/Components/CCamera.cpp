﻿#include "CCamera.h"
#include "Math/Math.h"

Transform& CCamera::GetTransform()
{
    _isViewMatrixDirty = true;
    _isViewProjectionMatrixDirty = true;

    return _transform;
}

const Transform& CCamera::GetTransform() const
{
    return _transform;
}

SharedObjectPtr<ViewportWidget> CCamera::GetViewport() const
{
    return _viewport.lock();
}

void CCamera::SetViewport(const SharedObjectPtr<ViewportWidget>& viewport)
{
    _viewport = viewport;
}

float CCamera::GetHorizontalFieldOfView() const
{
    return _horizontalFieldOfView;
}

void CCamera::SetHorizontalFieldOfView(float horizontalFieldOfView)
{
    _horizontalFieldOfView = horizontalFieldOfView;

    _isProjectionMatrixDirty = true;
    _isViewProjectionMatrixDirty = true;
}

float CCamera::GetNearClipPlane() const
{
    return _nearClipPlane;
}

void CCamera::SetNearClipPlane(float nearClipPlane)
{
    _nearClipPlane = nearClipPlane;

    _isProjectionMatrixDirty = true;
    _isViewProjectionMatrixDirty = true;
}

float CCamera::GetFarClipPlane() const
{
    return _farClipPlane;
}

void CCamera::SetFarClipPlane(float farClipPlane)
{
    _farClipPlane = farClipPlane;

    _isProjectionMatrixDirty = true;
    _isViewProjectionMatrixDirty = true;
}

float CCamera::GetAspectRatio() const
{
    return _aspectRatio;
}

void CCamera::SetAspectRatio(float aspectRatio)
{
    _aspectRatio = aspectRatio;

    _isProjectionMatrixDirty = true;
    _isViewProjectionMatrixDirty = true;
}

Matrix CCamera::GetViewMatrix()
{
    if (_isViewMatrixDirty)
    {
        const Vector3 location = _transform.GetWorldLocation();
        const Vector3 forward = _transform.GetForwardVector();
        // const Vector3 up = _transform.GetUpVector();

        // todo unlocked roll
        _viewMatrix = XMMatrixLookToLH(location, forward, Vector3(0.0f, 0.0f, 1.0f));
        _isViewMatrixDirty = false;
    }

    return _viewMatrix;
}

Matrix CCamera::GetProjectionMatrix()
{
    if (_isProjectionMatrixDirty)
    {
        _projectionMatrix = Math::CreatePerspectiveMatrix(
            _horizontalFieldOfView,
            _aspectRatio,
            _nearClipPlane,
            _farClipPlane);

        _isProjectionMatrixDirty = false;
    }

    return _projectionMatrix;
}

Matrix CCamera::GetViewProjectionMatrix()
{
    if (_isViewProjectionMatrixDirty)
    {
        _viewProjectionMatrix = GetViewMatrix() * GetProjectionMatrix();
        _isViewProjectionMatrixDirty = false;
    }

    return _viewProjectionMatrix;
}
