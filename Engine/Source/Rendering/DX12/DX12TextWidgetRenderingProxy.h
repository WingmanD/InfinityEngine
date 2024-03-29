﻿#pragma once

#include "DX12WidgetRenderingProxy.h"
#include "Delegate.h"
#include "SpriteBatch.h"
#include <d3d12.h>
#include <memory>

class DX12TextWidgetRenderingProxy : public DX12WidgetRenderingProxy
{
public:
    virtual void SetupDrawingInternal(ID3D12GraphicsCommandList* commandList) const override;

    // WidgetRenderingProxy
public:
    virtual bool Initialize() override;
    virtual void OnWindowChanged(const std::shared_ptr<Window>& oldWindow, const std::shared_ptr<Window>& newWindow) override;
    
private:
    std::unique_ptr<DirectX::SpriteBatch> _spriteBatch;
    DelegateHandle _viewportChangedHandle{};
};
