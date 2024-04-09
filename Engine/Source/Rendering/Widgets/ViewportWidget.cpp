#include "ViewportWidget.h"
#include "Engine/Subsystems/RenderingSubsystem.h"

void ViewportWidget::SetCamera(CCamera* camera)
{
    _camera = camera;
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
}
