#pragma once

#include "DX12WidgetRenderingProxy.h"

class DX12ViewportWidgetRenderingProxy : public DX12WidgetRenderingProxy
{
public:
    const D3D12_VIEWPORT& GetViewport() const;
    
protected:
    virtual DX12CommandList SetupDrawingInternal(DX12CommandList& commandList) const override;

protected:
    virtual void OnWidgetRectChanged() override;

private:
    D3D12_VIEWPORT _viewport{};
};
