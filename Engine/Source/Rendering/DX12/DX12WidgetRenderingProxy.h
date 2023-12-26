#pragma once

#include "Rendering/Widgets/WidgetRenderingProxy.h"
#include <d3d12.h>

class DX12WidgetRenderingProxy : public WidgetRenderingProxy
{
public:
    void SetupDrawing(ID3D12GraphicsCommandList* commandList) const;
    
protected:
    virtual void SetupDrawingInternal(ID3D12GraphicsCommandList* commandList) const;
};
