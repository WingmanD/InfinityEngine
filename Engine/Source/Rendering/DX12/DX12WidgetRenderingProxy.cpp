#include "DX12WidgetRenderingProxy.h"
#include "Rendering/StaticMesh.h"
#include "DX12StaticMeshRenderingData.h"
#include "Rendering/StaticMeshInstance.h"
#include "Rendering/Widgets/Widget.h"

void DX12WidgetRenderingProxy::SetupDrawing(ID3D12GraphicsCommandList* commandList) const
{
    const Widget& widget = GetOwningWidget();

    if (widget.IsVisible() && widget.GetMaterial() != nullptr)
    {
        SetupDrawingInternal(commandList);
    }

    for (const std::shared_ptr<Widget>& child : widget.GetChildren())
    {
        dynamic_cast<DX12WidgetRenderingProxy&>(child->GetRenderingProxy()).SetupDrawing(commandList);
    }
}

void DX12WidgetRenderingProxy::SetupDrawingInternal(ID3D12GraphicsCommandList* commandList) const
{
    commandList->RSSetScissorRects(1, &GetOwningWidget().GetRect());

    const StaticMeshInstance& meshInstance = GetOwningWidget().GetQuadMesh();

    StaticMeshRenderingData* renderingData = meshInstance.GetMesh()->GetRenderingData();
    if (renderingData->IsUploaded())    // todo remove this check after implementing SM instancing completely
    {
        static_cast<DX12StaticMeshRenderingData*>(renderingData)->SetupDrawing(commandList, meshInstance.GetMaterial());
    }
}
