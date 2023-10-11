﻿#pragma once

#include <d3d12.h>
#include <dxgi.h>

#include "PassKey.h"
#include "Rendering/StaticMesh.h"
#include "Rendering/Window.h"

class DX12StaticMesh;
struct DX12CommandList;
class DX12RenderTarget;
class DX12RenderingSubsystem;

using Microsoft::WRL::ComPtr;

class DX12Window : public Window
{
public:
    DX12Window(DX12RenderingSubsystem* renderingSubsystem, uint32 width, uint32 height, const std::wstring& title);

    DX12Window(const DX12Window&) = delete;
    DX12Window(DX12Window&&) = delete;
    DX12Window& operator=(const DX12Window&) = delete;
    DX12Window& operator=(DX12Window&&) = delete;
    
    virtual ~DX12Window() = default;

    virtual bool Initialize() override;
    void Render(PassKey<DX12RenderingSubsystem>);
    void Present(PassKey<DX12RenderingSubsystem>);

protected:
    struct FrameBuffer
    {
        ComPtr<ID3D12Resource> RenderTargetView;
        D3D12_CPU_DESCRIPTOR_HANDLE RTVDescriptorHandle;
    };
    
protected:
    DX12RenderingSubsystem* GetDX12RenderingSubsystem() const;

private:
    bool _needsResize = false;
    
    ComPtr<IDXGISwapChain> _swapChain;
    
    std::vector<FrameBuffer> _frameBuffers;
    int32 _currentFrameBufferIndex = 0;
    ComPtr<ID3D12Resource> _depthStencilBuffer;
    D3D12_CPU_DESCRIPTOR_HANDLE _depthStencilView;

    D3D12_VIEWPORT _fullWindowViewport;
    D3D12_RECT _fullWindowRect;

    std::shared_ptr<StaticMesh> _staticMeshTest = nullptr;

private:
    void ResizeImplementation(ID3D12GraphicsCommandList* commandList);
};
