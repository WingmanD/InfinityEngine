#include "DX12WidgetRenderingProxy.h"
#include "Rendering/StaticMesh.h"
#include "DX12StaticMeshRenderingData.h"
#include "Rendering/StaticMeshInstance.h"
#include "Rendering/Widgets/Widget.h"
#include <queue>

void DX12WidgetRenderingProxy::SetupDrawing(ID3D12GraphicsCommandList* commandList) const
{
    Widget& widget = GetOwningWidget();

    if (widget.IsVisible() && widget.GetMaterial() != nullptr)
    {
        SetupDrawingInternal(commandList);
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
            proxy.SetupDrawingInternal(commandList);
        }

        for (const std::shared_ptr<Widget>& childWidget : child->GetChildren())
        {
            queue.push(childWidget);
        }
    }
}

void DX12WidgetRenderingProxy::SetupDrawingInternal(ID3D12GraphicsCommandList* commandList) const
{
    commandList->RSSetScissorRects(1, &GetOwningWidget().GetRect());

    const StaticMeshInstance& meshInstance = GetOwningWidget().GetQuadMesh();

    StaticMeshRenderingData* renderingData = meshInstance.GetMesh()->GetRenderingData();
    if (renderingData->IsUploaded()) // todo remove this check after implementing SM instancing completely
    {
        static_cast<DX12StaticMeshRenderingData*>(renderingData)->SetupDrawing(commandList, meshInstance.GetMaterial());
    }
}
