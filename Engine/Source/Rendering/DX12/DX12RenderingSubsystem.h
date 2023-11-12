#pragma once

#include "Core.h"
#include "Engine/Subsystems/RenderingSubsystem.h"
#include <dxgi.h>
#include "d3dx12.h"
#include "DescriptorHeap.h"
#include "ThreadPool.h"

using Microsoft::WRL::ComPtr;

class DX12Window;

struct DX12CommandList
{
    ComPtr<ID3D12CommandAllocator> CommandAllocator;
    ComPtr<ID3D12GraphicsCommandList> CommandList;

    void Reset()
    {
        CommandAllocator->Reset();
        CommandList->Reset(CommandAllocator.Get(), nullptr);
    }
};

struct DX12CopyCommandList : public DX12CommandList
{
    std::vector<std::function<void()>> OnCompletedCallbacks;
};

class DX12RenderingSubsystem : public RenderingSubsystem, public std::enable_shared_from_this<DX12RenderingSubsystem>
{
public:
    static DX12RenderingSubsystem& Get();
    
    explicit DX12RenderingSubsystem() = default;
    
    DX12RenderingSubsystem(const DX12RenderingSubsystem&) = delete;
    DX12RenderingSubsystem(DX12RenderingSubsystem&&) = delete;
    DX12RenderingSubsystem& operator=(const DX12RenderingSubsystem&) = delete;
    DX12RenderingSubsystem& operator=(DX12RenderingSubsystem&&) = delete;
    ~DX12RenderingSubsystem() override = default;

    bool IsGPUReady() const;
    void WaitForGPU() const;

    ID3D12CommandQueue* GetCommandQueue() const;
    DX12CommandList RequestCommandList();
    void CloseCommandList(const DX12CommandList& commandList);

    ID3D12CommandQueue* GetCopyCommandQueue() const;
    DX12CopyCommandList RequestCopyCommandList();
    void ReturnCopyCommandList(DX12CopyCommandList& commandList);
    ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12GraphicsCommandList* commandList, const void* data, UINT64 byteSize, ComPtr<ID3D12Resource>& uploadBuffer) const;

    bool IsMSAAEnabled() const;
    uint32 GetMSAASampleCount() const;
    uint32 GetMSAAQuality() const;

    DXGI_FORMAT GetFrameBufferFormat() const;
    DXGI_FORMAT GetDepthStencilFormat() const;

    IDXGIFactory* GetDXGIFactory() const;
    ID3D12Device* GetDevice() const;

    DescriptorHeap& GetRTVHeap();
    DescriptorHeap& GetDSVHeap();
    const std::shared_ptr<DescriptorHeap>& GetCBVHeap();

    void AsyncOnGPUFenceEvent(std::function<void()>&& callback);
    void AsyncOnGPUCopyFenceEvent(std::function<void()>&& callback);

    // RenderingSubsystem
public:
    virtual bool Initialize() override;
    virtual void Shutdown() override;

    virtual void Tick(double deltaTime) override;

    virtual std::shared_ptr<Window> ConstructWindow(const std::wstring& title) override;
    
    virtual std::unique_ptr<StaticMeshRenderingData> CreateStaticMeshRenderingData() override;
    virtual std::unique_ptr<MaterialRenderingData> CreateMaterialRenderingData() override;
    virtual std::shared_ptr<Texture> CreateTexture(uint32 width, uint32 height) const override;
    virtual std::shared_ptr<RenderTarget> CreateRenderTarget(uint32 width, uint32 height) override;

    virtual void OnWindowDestroyed(Window* window) override;

    virtual uint32 GetBufferCount() const override;

private:
    uint32 _msaaSampleCount = 1;
    uint32 _msaaQuality = 1;

    uint32 _frameRateOverride = 0;

    std::vector<std::shared_ptr<DX12Window>> _windows;

    ThreadPool _asyncThreadPool{100};

    ComPtr<IDXGIFactory> _dxgiFactory = nullptr;
    ComPtr<ID3D12Device> _device;

    ComPtr<ID3D12Fence> _mainFence;
    uint64 _mainFenceValue = 0;

    ComPtr<ID3D12Fence> _copyFence;
    uint64 _copyFenceValue = 0;

    ComPtr<ID3D12CommandQueue> _commandQueue;
    std::vector<DX12CommandList> _availableCommandLists;
    std::vector<DX12CommandList> _closedCommandLists;

    ComPtr<ID3D12CommandQueue> _copyCommandQueue;
    LockFreeQueue<DX12CopyCommandList> _availableCopyCommandLists;
    LockFreeQueue<DX12CopyCommandList> _activeCopyCommandLists;

    DescriptorHeap _rtvHeap;
    DescriptorHeap _dsvHeap;
    std::shared_ptr<DescriptorHeap> _cbvHeap;

    uint32 _cbvSrvUavDescriptorSize = 0;

private:
    void LogAdapters();
    void LogAdapterOutputs(IDXGIAdapter* adapter);
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

    void HandleCopyLists();
};
