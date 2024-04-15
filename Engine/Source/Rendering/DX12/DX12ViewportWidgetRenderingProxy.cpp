#include "DX12ViewportWidgetRenderingProxy.h"
#include "DX12RenderingSubsystem.h"
#include "Rendering/Widgets/ViewportWidget.h"
#include "Rendering/Widgets/Widget.h"

const D3D12_VIEWPORT& DX12ViewportWidgetRenderingProxy::GetViewport() const
{
    return _viewport;
}

DX12CommandList DX12ViewportWidgetRenderingProxy::SetupDrawingInternal(DX12CommandList& commandList) const
{
    const ViewportWidget& widget = GetOwningWidget<ViewportWidget>();

    CCamera* camera = widget.GetCamera();
    if (camera == nullptr)
    {
        return commandList;
    }
    
    const std::shared_ptr<DX12Window> window = std::dynamic_pointer_cast<DX12Window>(widget.GetParentWindow());
    window->CloseCommandList(commandList);
    
    DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();
    renderingSubsystem.DrawScene(widget);

    return window->RequestCommandList();
}

void DX12ViewportWidgetRenderingProxy::OnWidgetRectChanged()
{
    const RECT rect = GetOwningWidget().GetRect();

    _viewport.TopLeftX = static_cast<float>(rect.left);
    _viewport.TopLeftY = static_cast<float>(rect.top);
    _viewport.Width = static_cast<float>(rect.right - rect.left);
    _viewport.Height = static_cast<float>(rect.bottom - rect.top);
    _viewport.MinDepth = 0.0f;
    _viewport.MaxDepth = 1.0f;
}
