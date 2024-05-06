#pragma once

#include "Rendering/DX12/DX12WidgetRenderingProxy.h"
#include "Rendering/ForwardPlusCore.h"

class DX12ViewportWidgetRenderingProxy : public DX12WidgetRenderingProxy
{
public:
    const D3D12_VIEWPORT& GetViewport() const;
    const RWStructuredBuffer<SimpleFrustum>& GetForwardPlusFrustumBuffer(PassKey<DX12RenderingSubsystem>) const;

    bool IsFrustumBufferDirty() const;
    void ClearFrustumBufferDirty(PassKey<DX12RenderingSubsystem>);
    
protected:
    virtual DX12CommandList SetupDrawingInternal(DX12CommandList& commandList) const override;

protected:
    virtual void OnWidgetRectChanged() override;

private:
    D3D12_VIEWPORT _viewport{};
    
    RWStructuredBuffer<SimpleFrustum> _forwardPlusFrustumBuffer{};
    bool _isFrustumBufferDirty = true;
};
