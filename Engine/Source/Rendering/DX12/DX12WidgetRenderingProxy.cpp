#include "DX12WidgetRenderingProxy.h"
#include "Rendering/StaticMesh.h"
#include "DX12StaticMeshRenderingData.h"
#include "Rendering/StaticMeshInstance.h"
#include "Rendering/Widgets/Widget.h"
#include <queue>

DX12CommandList DX12WidgetRenderingProxy::SetupDrawing(DX12CommandList& commandList) const
{
    Widget& widget = GetOwningWidget();

    DX12CommandList currentCommandList = commandList;
    if (widget.IsVisible() && widget.GetMaterial() != nullptr)
    {
        currentCommandList = SetupDrawingInternal(currentCommandList);
    }

    static std::queue<std::shared_ptr<Widget>> queue;
    queue.push(widget.SharedFromThis());

    while (!queue.empty())
    {
        const std::shared_ptr<Widget>& child = queue.front();
        queue.pop();

        DX12WidgetRenderingProxy& proxy = dynamic_cast<DX12WidgetRenderingProxy&>(child->GetRenderingProxy());
        if (child->IsVisible() && child->GetMaterial() != nullptr)
        {
            currentCommandList = proxy.SetupDrawingInternal(currentCommandList);
        }

        for (const std::shared_ptr<Widget>& childWidget : child->GetChildren())
        {
            queue.push(childWidget);
        }
    }

    return currentCommandList;
}

DX12CommandList DX12WidgetRenderingProxy::SetupDrawingInternal(DX12CommandList& commandList) const
{
    commandList.CommandList->RSSetScissorRects(1, &GetOwningWidget().GetRect());

    const StaticMeshInstance& meshInstance = GetOwningWidget().GetQuadMesh();

    const DX12StaticMeshRenderingData* renderingData = meshInstance.GetMesh()->GetRenderingData<DX12StaticMeshRenderingData>();
    if (renderingData->IsUploaded())
    {
        renderingData->DrawDirect(commandList.CommandList.Get(), meshInstance.GetMaterial());
    }

    return commandList;
}
