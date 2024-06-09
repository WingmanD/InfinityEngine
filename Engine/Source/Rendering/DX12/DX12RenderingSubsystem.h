#pragma once

#include "GraphicsMemory.h"
#include "ThreadPool.h"
#include "Containers/DynamicGPUBuffer2.h"
#include "Engine/Subsystems/RenderingSubsystem.h"
#include "Rendering/DX12/CullingWorkGraph.h"
#include "Rendering/DX12/DescriptorHeap.h"
#include "Rendering/DX12/DX12GPUBuffer.h"
#include "Rendering/DX12/DX12RenderingCore.h"

class ForwardPlusCullingComputeShader;
class CCamera;
class CompactSMInstancesComputeShader;
class DX12Window;
class InitializeForwardPlusComputeShader;
class InstanceBuffer;
class SortComputeShader;
class StaticMeshRenderingSystem;
class ViewportWidget;

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

    ID3D12CommandQueue* GetCopyCommandQueue() const;
    DX12CopyCommandList RequestCopyCommandList();
    void ReturnCopyCommandList(DX12CopyCommandList& commandList);
    
    ComPtr<ID3D12Resource> CreateDefaultBuffer(DX12GraphicsCommandList* commandList, const void* data,
                                               uint64 byteSize, ComPtr<ID3D12Resource>& uploadBuffer) const;
    
    void DrawScene(const ViewportWidget& viewport);

    bool IsMSAAEnabled() const;
    uint32 GetMSAASampleCount() const;
    uint32 GetMSAAQuality() const;

    DXGI_FORMAT GetFrameBufferFormat() const;
    DXGI_FORMAT GetDepthStencilFormat() const;

    IDXGIFactory* GetDXGIFactory() const;
    DX12Device* GetDevice() const;
    DXCompiler& GetD3DCompiler() const;
    IDxcUtils& GetDXCUtils() const;

    ComPtr<ID3DBlob> CompileDXILLibrary(const std::filesystem::path& libraryPath, const std::wstring& target, const DArray<DxcDefine>& defines = {}) const;

    DescriptorHeap& GetRTVHeap();
    DescriptorHeap& GetDSVHeap();
    const std::shared_ptr<DescriptorHeap>& GetCBVHeap();
    const std::shared_ptr<DescriptorHeap>& GetSRVHeap();

    uint32 GetCBVSRVUAVDescriptorSize() const;

    DirectX::GraphicsMemory& GetGraphicsMemory() const;

    void AsyncOnGPUFenceEvent(std::function<void()>&& callback);
    void AsyncOnGPUCopyFenceEvent(std::function<void()>&& callback);

    template <typename T>
    void UpdateDynamicBuffer(DynamicGPUBuffer2<T>& buffer, DX12GraphicsCommandList* commandList) const
    {
        const DX12GPUBuffer& dx12Buffer = buffer.template GetBuffer<DX12GPUBuffer>();
        if (buffer.IsCompletelyDirty())
        {
            dx12Buffer.Update(commandList);
        }
        else
        {
            dx12Buffer.Update(commandList, buffer.GetDirtyIndices());
        }
    }

    // RenderingSubsystem
public:
    virtual bool Initialize() override;
    virtual void Shutdown() override;

    virtual void Tick(double deltaTime) override;

    virtual std::shared_ptr<Window> ConstructWindow(const std::wstring& title) override;
    virtual void ForEachWindow(std::function<bool(Window*)> callback) override;

    virtual std::unique_ptr<StaticMeshRenderingData> CreateStaticMeshRenderingData() override;
    virtual std::unique_ptr<MaterialRenderingData> CreateMaterialRenderingData() override;
    virtual std::unique_ptr<MaterialParameterRenderingData> CreateMaterialParameterRenderingData() override;
    virtual SharedObjectPtr<Texture> CreateTexture(uint32 width, uint32 height) const override;
    virtual std::shared_ptr<RenderTarget> CreateRenderTarget(uint32 width, uint32 height) override;
    virtual std::unique_ptr<WidgetRenderingProxy> CreateDefaultWidgetRenderingProxy() override;
    virtual std::unique_ptr<WidgetRenderingProxy> CreateTextWidgetRenderingProxy() override;
    virtual std::unique_ptr<WidgetRenderingProxy> CreateViewportWidgetRenderingProxy() override;

    virtual void RegisterStaticMeshRenderingSystem(StaticMeshRenderingSystem* system) override;
    virtual void UnregisterStaticMeshRenderingSystem(StaticMeshRenderingSystem* system) override;

    virtual void InitializeMaterialInstanceBuffer(DynamicGPUBuffer<MaterialParameter>& instanceBuffer, Type* type) override;
    virtual std::unique_ptr<GPUBuffer> CreateBuffer(GPUBuffer::EType type) override;

    virtual void OnWindowDestroyed(Window* window) override;

    virtual uint32 GetBufferCount() const override;

private:
    uint32 _msaaSampleCount = 1;
    uint32 _msaaQuality = 1;

    uint32 _frameRateOverride = 0;

    std::vector<std::shared_ptr<DX12Window>> _windows;

    ComPtr<IDXGIFactory> _dxgiFactory = nullptr;
    ComPtr<DX12Device> _device;
    ComPtr<DXCompiler> _compiler;
    ComPtr<IDxcUtils> _dxcUtils;

    ComPtr<ID3D12Fence> _mainFence;
    uint64 _mainFenceValue = 0;

    ComPtr<ID3D12Fence> _copyFence;
    uint64 _copyFenceValue = 0;

    ComPtr<ID3D12CommandQueue> _commandQueue;

    ComPtr<ID3D12CommandQueue> _copyCommandQueue;
    LockFreeQueue<DX12CopyCommandList> _availableCopyCommandLists;
    LockFreeQueue<DX12CopyCommandList> _activeCopyCommandLists;

    DescriptorHeap _rtvHeap;
    DescriptorHeap _dsvHeap;
    std::shared_ptr<DescriptorHeap> _cbvHeap;
    std::shared_ptr<DescriptorHeap> _srvHeap;

    uint32 _cbvSrvUavDescriptorSize = 0;

    DirectX::GraphicsMemory* _graphicsMemory;

    DArray<StaticMeshRenderingSystem*> _staticMeshRenderingSystems;

    CullingWorkGraph _cullingWorkGraph;
    SharedObjectPtr<SortComputeShader> _sortCS;
    SharedObjectPtr<CompactSMInstancesComputeShader> _compactSMInstancesCS;
    SharedObjectPtr<InitializeForwardPlusComputeShader> _initializeForwardPlusCS;
    SharedObjectPtr<ForwardPlusCullingComputeShader> _forwardPlusCullingCS;

private:
    void LogAdapters();
    void LogAdapterOutputs(IDXGIAdapter* adapter);
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

    void HandleCopyLists();
};
