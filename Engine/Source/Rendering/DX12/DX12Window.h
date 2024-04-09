#pragma once

#include "PassKey.h"
#include "Rendering/DX12/DX12RenderingCore.h"
#include "Rendering/StaticMesh.h"
#include "Rendering/Window.h"
#include <dxgi.h>
#include <d3d12.h>

class ViewportWidget;
using Microsoft::WRL::ComPtr;

class DX12RenderingSubsystem;

class DX12Window : public Window
{
public:
    Delegate<const D3D12_VIEWPORT&> OnViewportChanged;
    
public:
    DX12Window(uint32 width, uint32 height, const std::wstring& title);

    virtual bool Initialize() override;
    void Render(PassKey<DX12RenderingSubsystem>);
    void Present(PassKey<DX12RenderingSubsystem>);
    void EndFrame(PassKey<DX12RenderingSubsystem>);

    const D3D12_VIEWPORT& GetViewport() const;

    DX12CommandList RequestCommandList();
    DX12CommandList RequestCommandList(const ViewportWidget& viewport);
    void CloseCommandList(DX12CommandList& commandList);
    void ExecuteCommandLists();

protected:
    struct FrameBuffer
    {
        ComPtr<ID3D12Resource> RenderTargetView;
        D3D12_CPU_DESCRIPTOR_HANDLE RTVDescriptorHandle;
    };
    
protected:
    DX12RenderingSubsystem& GetDX12RenderingSubsystem() const;

private:
    bool _needsResize = false;
    
    ComPtr<IDXGISwapChain> _swapChain;
    
    std::vector<FrameBuffer> _frameBuffers;
    int32 _currentFrameBufferIndex = 0;
    ComPtr<ID3D12Resource> _depthStencilBuffer;
    D3D12_CPU_DESCRIPTOR_HANDLE _depthStencilView{};

    D3D12_VIEWPORT _fullWindowViewport{};
    D3D12_RECT _fullWindowRect{};

    std::shared_ptr<StaticMesh> _staticMeshTest = nullptr;

    DArray<DX12CommandList> _availableCommandLists;
    DArray<DX12CommandList> _closedCommandLists;

private:
    void ResizeImplementation(DX12GraphicsCommandList* commandList);
    DX12CommandList RequestCommandListImplementation();
    DX12CommandList RequestUninitializedCommandList();
};
