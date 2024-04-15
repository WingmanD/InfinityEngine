#include "ViewportWidget.h"

#include "ECS/Components/CCamera.h"
#include "Engine/Subsystems/RenderingSubsystem.h"

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
