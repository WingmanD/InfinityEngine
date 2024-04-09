#pragma once

#include "Rendering/Widgets/WidgetRenderingProxy.h"
#include "Rendering/DX12/DX12RenderingCore.h"

class DX12WidgetRenderingProxy : public WidgetRenderingProxy
{
public:
    DX12CommandList SetupDrawing(DX12CommandList& commandList) const;
    
protected:
    virtual DX12CommandList SetupDrawingInternal(DX12CommandList& commandList) const;
};
