#pragma once

#include "d3d12.h"
#include <wrl/client.h>
#include "Rendering/RenderTarget.h"

using Microsoft::WRL::ComPtr;

class DX12RenderingSubsystem;

class DX12RenderTarget : public RenderTarget
{
public:
    DX12RenderTarget(const std::shared_ptr<DX12RenderingSubsystem>& renderingSubsystem, uint32 width, uint32 height);
    ~DX12RenderTarget() override;

    ComPtr<ID3D12Resource>& GetRenderTargetView();
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetRenderTargetViewDescriptorHandle() const;

    // RenderTarget
public:
    virtual void Resize(uint32 width, uint32 height) override;

protected:
    void SetRenderTargetViewDescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle);

private:
    std::weak_ptr<DX12RenderingSubsystem> _renderingSubsystem;
    
    ComPtr<ID3D12Resource> _renderTargetView;
    D3D12_CPU_DESCRIPTOR_HANDLE _renderTargetViewDescriptorHandle = {};
};
