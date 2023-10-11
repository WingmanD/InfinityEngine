#include "DX12RenderTarget.h"
#include "DX12RenderingSubsystem.h"

DX12RenderTarget::DX12RenderTarget(const std::shared_ptr<DX12RenderingSubsystem>& renderingSubsystem, uint32 width,
                                   uint32 height) : RenderTarget(width, height), _renderingSubsystem(renderingSubsystem)
{
    _renderTargetViewDescriptorHandle = renderingSubsystem->GetRTVHeap().RequestHeapResourceHandle();

    D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
    renderTargetViewDesc.Format = renderingSubsystem->GetFrameBufferFormat();
    renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    renderTargetViewDesc.Texture2D.MipSlice = 0;
    renderTargetViewDesc.Texture2D.PlaneSlice = 0;

    renderingSubsystem->GetDevice()->CreateRenderTargetView(
        _renderTargetView.Get(),
        &renderTargetViewDesc,
        _renderTargetViewDescriptorHandle);
}

DX12RenderTarget::~DX12RenderTarget()
{
    if (_renderTargetView != nullptr)
    {
        _renderTargetView.Reset();
        _renderingSubsystem.lock()->GetRTVHeap().FreeHeapResourceHandle(_renderTargetViewDescriptorHandle);
    }
}

ComPtr<ID3D12Resource>& DX12RenderTarget::GetRenderTargetView()
{
    return _renderTargetView;
}

const D3D12_CPU_DESCRIPTOR_HANDLE& DX12RenderTarget::GetRenderTargetViewDescriptorHandle() const
{
    return _renderTargetViewDescriptorHandle;
}

void DX12RenderTarget::Resize(uint32 width, uint32 height)
{
    std::shared_ptr<DX12RenderingSubsystem> renderingSubsystem = _renderingSubsystem.lock();
    if (_renderTargetView != nullptr)
    {
        _renderTargetView.Reset();
        renderingSubsystem->GetRTVHeap().FreeHeapResourceHandle(_renderTargetViewDescriptorHandle);
    }

    _renderTargetViewDescriptorHandle = renderingSubsystem->GetRTVHeap().RequestHeapResourceHandle();

    renderingSubsystem->GetDevice()->CreateRenderTargetView(
        _renderTargetView.Get(),
        nullptr,
        _renderTargetViewDescriptorHandle);
}

void DX12RenderTarget::SetRenderTargetViewDescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    _renderTargetViewDescriptorHandle = handle;
}
