#include "ViewportWidget.h"
#include "ECS/Components/CCamera.h"
#include "Engine/Subsystems/InputSubsystem.h"
#include "Engine/Subsystems/RenderingSubsystem.h"

ViewportWidget::ViewportWidget()
{
    EnableInputCompatibility(EWidgetInputCompatibility::LeftClick | EWidgetInputCompatibility::Focus);
}

void ViewportWidget::SetCamera(CCamera* camera)
{
    _camera = camera;

    if (_camera != nullptr)
    {
        const Vector2 screenSpaceSize = GetScreenSize();
        _camera->SetAspectRatio(screenSpaceSize.x / screenSpaceSize.y);
    }
}

CCamera* ViewportWidget::GetCamera() const
{
    return _camera;
}

void ViewportWidget::CaptureMouse()
{
    if (_isMouseCaptured)
    {
        return;
    }

    _isMouseCaptured = true;

    InputSubsystem& inputSubsystem = InputSubsystem::Get();
    inputSubsystem.GetMouse().SetVisible(false);
    inputSubsystem.SetMouseCaptured(true, {});
}

void ViewportWidget::ReleaseMouse()
{
    if (!_isMouseCaptured)
    {
        return;
    }

    _isMouseCaptured = false;

    ReleaseCapture();
    ClipCursor(nullptr);

    InputSubsystem& inputSubsystem = InputSubsystem::Get();
    inputSubsystem.GetMouse().SetVisible(true);
    inputSubsystem.SetMouseCaptured(false, {});
}

Vector3 ViewportWidget::GetMouseDirectionWS() const
{
    const Vector2 mousePosition = InputSubsystem::Get().GetMousePosition();
    const RECT& screenSpaceSize = GetRect();
    
    const Vector2 normalizedMousePosition = {
        2.0f * (mousePosition.x - screenSpaceSize.left) / (screenSpaceSize.right - screenSpaceSize.left) - 1.0f,
        1.0f - 2.0f * (mousePosition.y - screenSpaceSize.top) / (screenSpaceSize.bottom - screenSpaceSize.top)
    };
    
    CCamera* camera = GetCamera();
    const Matrix invProjView = camera->GetViewProjectionMatrix().Invert();
    
    const Vector3 nearPointNDC = {normalizedMousePosition.x, normalizedMousePosition.y, 0.0f};
    
    const Vector3 nearPointWS = Vector3::Transform(nearPointNDC, invProjView);
    
    Vector3 direction = nearPointWS - camera->GetTransform().GetWorldLocation();
    direction.Normalize();
    
    return direction;
}

void ViewportWidget::SetCaptureMouseOnClick(bool value)
{
    _shouldCaptureMouseOnClick = value;
}

bool ViewportWidget::ShouldCaptureMouseOnClick() const
{
    return _shouldCaptureMouseOnClick;
}

bool ViewportWidget::Initialize()
{
    if (!CanvasPanel::Initialize())
    {
        return false;
    }

    SetCollisionEnabled(true);

    return true;
}

bool ViewportWidget::InitializeRenderingProxy()
{
    RenderingProxy = RenderingSubsystem::Get().CreateViewportWidgetRenderingProxy();
    if (RenderingProxy == nullptr)
    {
        return false;
    }

    RenderingProxy->SetWidget(this);

    return RenderingProxy->Initialize();
}

void ViewportWidget::OnWidgetRectChanged()
{
    CanvasPanel::OnWidgetRectChanged();

    RenderingProxy->OnWidgetRectChanged();

    if (_camera != nullptr)
    {
        const Vector2 screenSpaceSize = GetScreenSize();
        _camera->SetAspectRatio(screenSpaceSize.x / screenSpaceSize.y);
    }
}

bool ViewportWidget::OnReleasedInternal()
{
    CanvasPanel::OnReleasedInternal();

    if (!ShouldCaptureMouseOnClick())
    {
        SetFocused(false);
    }

    return true;
}

void ViewportWidget::OnFocusChangedInternal(bool focused)
{
    CanvasPanel::OnFocusChangedInternal(focused);

    if (focused)
    {
        if (GetCamera() != nullptr)
        {
            CaptureMouse();
        }
    }
    else
    {
        ReleaseMouse();
    }
}
