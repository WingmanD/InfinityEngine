#include "DX12ViewportWidgetRenderingProxy.h"
#include "DX12RenderingSubsystem.h"
#include "Rendering/Widgets/ViewportWidget.h"
#include "Rendering/Widgets/Widget.h"

const D3D12_VIEWPORT& DX12ViewportWidgetRenderingProxy::GetViewport() const
{
    return _viewport;
}

const RWStructuredBuffer<SimpleFrustum>& DX12ViewportWidgetRenderingProxy::GetForwardPlusFrustumBuffer(PassKey<DX12RenderingSubsystem>) const
{
    return _forwardPlusFrustumBuffer;
}

bool DX12ViewportWidgetRenderingProxy::IsFrustumBufferDirty() const
{
    return _isFrustumBufferDirty;
}

void DX12ViewportWidgetRenderingProxy::ClearFrustumBufferDirty(PassKey<DX12RenderingSubsystem>)
{
    _isFrustumBufferDirty = false;
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

    const uint32 width = static_cast<uint32>(rect.right - rect.left);
    const uint32 height = static_cast<uint32>(rect.bottom - rect.top);

    _viewport.TopLeftX = static_cast<float>(rect.left);
    _viewport.TopLeftY = static_cast<float>(rect.top);
    _viewport.Width = static_cast<float>(width);
    _viewport.Height = static_cast<float>(height);
    _viewport.MinDepth = 0.0f;
    _viewport.MaxDepth = 1.0f;

    constexpr uint32 tileFrustumCount = Math::Square(ERenderingSettings::ForwardPlusTileDim);
    const uint32 numFrustums = width * height / tileFrustumCount;

    if (numFrustums > 0)
    {
        DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();
        _forwardPlusFrustumBuffer.~RWStructuredBuffer();
        new (&_forwardPlusFrustumBuffer) RWStructuredBuffer<SimpleFrustum>();
        _forwardPlusFrustumBuffer.Initialize(numFrustums, *renderingSubsystem.GetDevice(), renderingSubsystem.GetCBVHeap());
        _forwardPlusFrustumBuffer.GetBuffer()->SetName(L"ForwardPlusFrustumBuffer");

        _isFrustumBufferDirty = true;
    }
}
