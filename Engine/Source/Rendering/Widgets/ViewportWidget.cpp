#include "ViewportWidget.h"
#include "ECS/Components/CCamera.h"
#include "Engine/Subsystems/InputSubsystem.h"
#include "Engine/Subsystems/RenderingSubsystem.h"

ViewportWidget::ViewportWidget()
{
    EnableInputCompatibility(EWidgetInputCompatibility::LeftClick);
}

void ViewportWidget::SetCamera(CCamera* camera)
{
    _camera = camera;

    const Vector2 screenSpaceSize = GetScreenSize();
    _camera->SetAspectRatio(screenSpaceSize.x / screenSpaceSize.y);
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

    SetCapture(GetParentWindow()->GetHandle());
    ClipCursor(&GetRect());

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
