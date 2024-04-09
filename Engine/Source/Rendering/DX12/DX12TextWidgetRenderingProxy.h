#pragma once

#include "DX12WidgetRenderingProxy.h"
#include "Delegate.h"
#include "SpriteBatch.h"
#include <memory>

class DX12TextWidgetRenderingProxy : public DX12WidgetRenderingProxy
{
public:
    virtual DX12CommandList SetupDrawingInternal(DX12CommandList& commandList) const override;

    // WidgetRenderingProxy
public:
    virtual bool Initialize() override;
    virtual void OnWindowChanged(const std::shared_ptr<Window>& oldWindow, const std::shared_ptr<Window>& newWindow) override;
    
private:
    std::unique_ptr<DirectX::SpriteBatch> _spriteBatch;
    DelegateHandle _viewportChangedHandle{};
};
