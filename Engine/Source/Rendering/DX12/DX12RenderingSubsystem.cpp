﻿#include "DX12RenderingSubsystem.h"
#include "Core.h"
#include "d3dx12.h"
#include "DX12MaterialParameterRenderingData.h"
#include "DX12MaterialRenderingData.h"
#include "DX12RenderTarget.h"
#include "DX12Shader.h"
#include "DX12StaticMeshRenderingData.h"
#include "DX12TextWidgetRenderingProxy.h"
#include "DX12WidgetRenderingProxy.h"
#include "DX12Window.h"
#include "ThreadPool.h"
#include "Engine/Engine.h"
#include "Rendering/Window.h"
#include <dxgi1_2.h>
#include <string>
#include <vector>

DX12RenderingSubsystem& DX12RenderingSubsystem::Get()
{
    return static_cast<DX12RenderingSubsystem&>(*Engine::Get().GetRenderingSubsystem());
}

bool DX12RenderingSubsystem::IsGPUReady() const
{
    return _mainFence->GetCompletedValue() >= _mainFenceValue;
}

void DX12RenderingSubsystem::WaitForGPU() const
{
    if (!IsGPUReady())
    {
        const HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

        ThrowIfFailed(_mainFence->SetEventOnCompletion(_mainFenceValue, eventHandle));

        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

ID3D12CommandQueue* DX12RenderingSubsystem::GetCommandQueue() const
{
    return _commandQueue.Get();
}

DX12CommandList DX12RenderingSubsystem::RequestCommandList()
{
    if (_availableCommandLists.empty())
    {
        DX12CommandList commandList;

        ThrowIfFailed(_device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(commandList.CommandAllocator.GetAddressOf())));

        ThrowIfFailed(_device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            commandList.CommandAllocator.Get(),
            nullptr,
            IID_PPV_ARGS(commandList.CommandList.GetAddressOf())));

        return commandList;
    }

    // todo this part must be thread safe - use LockFreeQueue
    DX12CommandList commandList = _availableCommandLists.back();
    _availableCommandLists.pop_back();

    return commandList;
}

void DX12RenderingSubsystem::CloseCommandList(const DX12CommandList& commandList)
{
    commandList.CommandList->Close();
    _closedCommandLists.push_back(commandList);
}

ID3D12CommandQueue* DX12RenderingSubsystem::GetCopyCommandQueue() const
{
    return _copyCommandQueue.Get();
}

DX12CopyCommandList DX12RenderingSubsystem::RequestCopyCommandList()
{
    DX12CopyCommandList commandList;

    // todo multiple threads can write to a command list - use that instead of creating a new one
    if (_activeCopyCommandLists.Dequeue(commandList))
    {
        return commandList;
    }

    if (_availableCopyCommandLists.Dequeue(commandList))
    {
        return commandList;
    }

    ThrowIfFailed(_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_COPY,
        IID_PPV_ARGS(commandList.CommandAllocator.GetAddressOf())));

    ThrowIfFailed(_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_COPY,
        commandList.CommandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(commandList.CommandList.GetAddressOf())));

    static int32 id = 0;
    const std::wstring name = L"Copy Command List " + std::to_wstring(id++);
    commandList.CommandList->SetName(name.c_str());

    return commandList;
}

void DX12RenderingSubsystem::ReturnCopyCommandList(DX12CopyCommandList& commandList)
{
    _activeCopyCommandLists.Enqueue(std::move(commandList));
}

ComPtr<ID3D12Resource> DX12RenderingSubsystem::CreateDefaultBuffer(
    ID3D12GraphicsCommandList* commandList,
    const void* data,
    uint64 byteSize,
    ComPtr<ID3D12Resource>& uploadBuffer) const
{
    ComPtr<ID3D12Resource> buffer;

    {
        const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
        const CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
        ThrowIfFailed(_device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(buffer.GetAddressOf())));
    }

    {
        const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
        const CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
        ThrowIfFailed(_device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(uploadBuffer.GetAddressOf())));
    }


    D3D12_SUBRESOURCE_DATA subResourceData;
    subResourceData.pData = data;
    subResourceData.RowPitch = static_cast<int64>(byteSize);
    subResourceData.SlicePitch = subResourceData.RowPitch;

    {
        const CD3DX12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition(
            buffer.Get(),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_COPY_DEST);
        commandList->ResourceBarrier(1,
                                     &transition);
    }

    {
        const CD3DX12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition(
            buffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_COMMON);
        UpdateSubresources<1>(commandList, buffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
        commandList->ResourceBarrier(1,
                                     &transition);
    }

    return buffer;
}

bool DX12RenderingSubsystem::IsMSAAEnabled() const
{
    return _msaaSampleCount > 1;
}

uint32 DX12RenderingSubsystem::GetMSAASampleCount() const
{
    return _msaaSampleCount;
}

uint32 DX12RenderingSubsystem::GetMSAAQuality() const
{
    return _msaaQuality;
}

DXGI_FORMAT DX12RenderingSubsystem::GetFrameBufferFormat() const
{
    return DXGI_FORMAT_R8G8B8A8_UNORM;
}

DXGI_FORMAT DX12RenderingSubsystem::GetDepthStencilFormat() const
{
    return DXGI_FORMAT_D24_UNORM_S8_UINT;
}

IDXGIFactory* DX12RenderingSubsystem::GetDXGIFactory() const
{
    return _dxgiFactory.Get();
}

ID3D12Device* DX12RenderingSubsystem::GetDevice() const
{
    return _device.Get();
}

DescriptorHeap& DX12RenderingSubsystem::GetRTVHeap()
{
    return _rtvHeap;
}

DescriptorHeap& DX12RenderingSubsystem::GetDSVHeap()
{
    return _dsvHeap;
}

const std::shared_ptr<DescriptorHeap>& DX12RenderingSubsystem::GetCBVHeap()
{
    return _cbvHeap;
}

const std::shared_ptr<DescriptorHeap>& DX12RenderingSubsystem::GetSRVHeap()
{
    return _srvHeap;
}

DirectX::GraphicsMemory& DX12RenderingSubsystem::GetGraphicsMemory() const
{
    return *_graphicsMemory;
}

void DX12RenderingSubsystem::AsyncOnGPUFenceEvent(std::function<void()>&& callback)
{
    const HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

    ++_mainFenceValue;
    ThrowIfFailed(_commandQueue->Signal(_mainFence.Get(), _mainFenceValue));

    ThrowIfFailed(_mainFence->SetEventOnCompletion(_mainFenceValue, eventHandle));

    Engine::Get().GetWaitThreadPool().EnqueueTask([eventHandle, callback]()
    {
        WaitForSingleObjectEx(eventHandle, INFINITE, FALSE);
        CloseHandle(eventHandle);

        callback();
    });
}

void DX12RenderingSubsystem::AsyncOnGPUCopyFenceEvent(std::function<void()>&& callback)
{
    const HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

    ++_copyFenceValue;
    ThrowIfFailed(_copyCommandQueue->Signal(_copyFence.Get(), _copyFenceValue));

    ThrowIfFailed(_copyFence->SetEventOnCompletion(_copyFenceValue, eventHandle));

    Engine::Get().GetWaitThreadPool().EnqueueTask([eventHandle, callback]()
    {
        WaitForSingleObjectEx(eventHandle, INFINITE, FALSE);
        CloseHandle(eventHandle);

        callback();
    });
}

bool DX12RenderingSubsystem::Initialize()
{
    if (!RenderingSubsystem::Initialize())
    {
        return false;
    }

#if DEBUG
    {
        ComPtr<ID3D12Debug> debugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
    }
#endif
    ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&_dxgiFactory)));

    ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&_device)));

    ThrowIfFailed(_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_mainFence)));
    ThrowIfFailed(_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_copyFence)));

    _cbvSrvUavDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        ThrowIfFailed(_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue)));
    }

    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        ThrowIfFailed(_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_copyCommandQueue)));
    }

    // LogAdapters();

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = 100;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    _rtvHeap = DescriptorHeap(_device.Get(), rtvHeapDesc);

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 10;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    _dsvHeap = DescriptorHeap(_device.Get(), dsvHeapDesc);

    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = 1024;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;
    _cbvHeap = std::make_shared<DescriptorHeap>(_device.Get(), cbvHeapDesc);

    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc;
    srvHeapDesc.NumDescriptors = 1024;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    srvHeapDesc.NodeMask = 0;
    _srvHeap = std::make_shared<DescriptorHeap>(_device.Get(), srvHeapDesc);

    DX12CommandList commandList;

    ThrowIfFailed(_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(commandList.CommandAllocator.GetAddressOf())));

    ThrowIfFailed(_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandList.CommandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(commandList.CommandList.GetAddressOf())));
    _availableCommandLists.push_back(commandList);

    _graphicsMemory = new DirectX::DX12::GraphicsMemory(_device.Get());

    return true;
}

void DX12RenderingSubsystem::Shutdown()
{
    RenderingSubsystem::Shutdown();
}

void DX12RenderingSubsystem::Tick(double deltaTime)
{
    GetEventQueue().ProcessEvents();

    for (std::shared_ptr<DX12Window>& window : _windows)
    {
        window->Tick(deltaTime);
    }

    HandleCopyLists();

    // todo multiple descriptor heaps
    ID3D12DescriptorHeap* descriptorHeaps[] = {_cbvHeap->GetHeap().Get()};
    _availableCommandLists[0].CommandList->SetDescriptorHeaps(1, descriptorHeaps);

    for (std::shared_ptr<DX12Window>& window : _windows)
    {
        // the reason we can't use the thread pool currently is the command list request/close that is not multi-threaded, also, command lists must be executed in order of windows
        // GetEngine()->GetThreadPool().EnqueueTask([window, weakThis]()
        // {
        //     
        // });

        window->Render({});

        static std::vector<ID3D12CommandList*> commandLists;
        commandLists.clear();

        for (const DX12CommandList& commandList : _closedCommandLists)
        {
            commandLists.push_back(commandList.CommandList.Get());
        }

        _commandQueue->ExecuteCommandLists(static_cast<uint32>(commandLists.size()), commandLists.data());

        window->Present({});

        ++_mainFenceValue;
        ThrowIfFailed(_commandQueue->Signal(_mainFence.Get(), _mainFenceValue));

        WaitForGPU();
        while (!_closedCommandLists.empty())
        {
            DX12CommandList& commandList = _closedCommandLists.back();
            commandList.Reset();

            _availableCommandLists.push_back(commandList);
            _closedCommandLists.pop_back();
        }
    }

    _graphicsMemory->Commit(GetCommandQueue());
    _graphicsMemory->Commit(GetCopyCommandQueue());
}

std::shared_ptr<Window> DX12RenderingSubsystem::ConstructWindow(const std::wstring& title)
{
    std::shared_ptr<DX12Window> window = std::make_shared<DX12Window>(1280, 720, title);
    if (window->Initialize())
    {
        _windows.push_back(window);

        return window;
    }

    return nullptr;
}

void DX12RenderingSubsystem::ForEachWindow(std::function<bool(Window*)> callback)
{
    for (const std::shared_ptr<DX12Window>& dx12Window : _windows)
    {
        if (!callback(dx12Window.get()))
        {
            break;
        }
    } 
}

std::unique_ptr<StaticMeshRenderingData> DX12RenderingSubsystem::CreateStaticMeshRenderingData()
{
    return std::make_unique<DX12StaticMeshRenderingData>();
}

std::unique_ptr<MaterialRenderingData> DX12RenderingSubsystem::CreateMaterialRenderingData()
{
    return std::make_unique<DX12MaterialRenderingData>();
}

std::unique_ptr<MaterialParameterRenderingData> DX12RenderingSubsystem::CreateMaterialParameterRenderingData()
{
    return std::make_unique<DX12MaterialParameterRenderingData>();
}

std::shared_ptr<Texture> DX12RenderingSubsystem::CreateTexture(uint32 width, uint32 height) const
{
    return nullptr;
}

std::shared_ptr<RenderTarget> DX12RenderingSubsystem::CreateRenderTarget(uint32 width, uint32 height)
{
    return std::make_shared<DX12RenderTarget>(shared_from_this(), width, height);
}

std::unique_ptr<WidgetRenderingProxy> DX12RenderingSubsystem::CreateDefaultWidgetRenderingProxy()
{
    return std::make_unique<DX12WidgetRenderingProxy>();
}

std::unique_ptr<WidgetRenderingProxy> DX12RenderingSubsystem::CreateTextWidgetRenderingProxy()
{
    return std::make_unique<DX12TextWidgetRenderingProxy>();
}

void DX12RenderingSubsystem::OnWindowDestroyed(Window* window)
{
    const auto it = std::ranges::find_if(_windows,
                                         [window](const std::shared_ptr<Window>& w)
                                         {
                                             return w.get() == window;
                                         });

    if (it != _windows.end())
    {
        _windows.erase(it);
    }

    if (_windows.empty())
    {
        Engine::Get().RequestExit();
    }
}

uint32 DX12RenderingSubsystem::GetBufferCount() const
{
    // todo read from config;
    return 2;
}

void DX12RenderingSubsystem::LogAdapters()
{
    uint32 i = 0;
    IDXGIAdapter* currentAdapter = nullptr;
    std::vector<IDXGIAdapter*> adapterList;

    while (_dxgiFactory->EnumAdapters(i, &currentAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        currentAdapter->GetDesc(&desc);

        std::wstring text = L"Display Adapter: ";
        text += desc.Description;
        text += L"\n";

        OutputDebugString(text.c_str());

        adapterList.push_back(currentAdapter);

        ++i;
    }

    for (IDXGIAdapter* adapter : adapterList)
    {
        LogAdapterOutputs(adapter);
        adapter->Release();
    }
}

void DX12RenderingSubsystem::LogAdapterOutputs(IDXGIAdapter* adapter)
{
    uint32 i = 0;
    IDXGIOutput* output = nullptr;

    while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);

        std::wstring text = L"Display Output: ";
        text += desc.DeviceName;
        text += L"\n";

        OutputDebugString(text.c_str());

        LogOutputDisplayModes(output, DXGI_FORMAT_B8G8R8A8_UNORM);

        output->Release();

        ++i;
    }
}

void DX12RenderingSubsystem::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
    uint32 count = 0;
    constexpr uint32 flags = 0;

    output->GetDisplayModeList(format, flags, &count, nullptr);

    std::vector<DXGI_MODE_DESC> modes(count);
    output->GetDisplayModeList(format, flags, &count, modes.data());

    for (const DXGI_MODE_DESC& mode : modes)
    {
        const uint32 n = mode.RefreshRate.Numerator;
        const uint32 d = mode.RefreshRate.Denominator;
        std::wstring text =
            L"Height = " + std::to_wstring(mode.Height) + L" " +
            L"Height = " + std::to_wstring(mode.Height) + L" " +
            L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
            L"\n";

        OutputDebugString(text.c_str());
    }
}

void DX12RenderingSubsystem::HandleCopyLists()
{
    if (_activeCopyCommandLists.IsEmpty())
    {
        return;
    }

    std::vector<DX12CopyCommandList> currentCommandLists;
    std::vector<ID3D12CommandList*> copyCommandLists;

    DX12CopyCommandList copyCommandList;
    while (_activeCopyCommandLists.Dequeue(copyCommandList))
    {
        copyCommandList.CommandList->Close();

        copyCommandLists.push_back(copyCommandList.CommandList.Get());

        currentCommandLists.push_back(std::move(copyCommandList));
    }

    _copyCommandQueue->ExecuteCommandLists(static_cast<uint32>(copyCommandLists.size()), copyCommandLists.data());
    _copyCommandQueue->Signal(_copyFence.Get(), ++_copyFenceValue);

    AsyncOnGPUCopyFenceEvent([currentCommandLists, this]()
    {
        if (Engine::Get().GetRenderingSubsystem() == nullptr)
        {
            return;
        }

        for (DX12CopyCommandList commandList : currentCommandLists)
        {
            for (const std::function<void()>& callback : commandList.OnCompletedCallbacks)
            {
                callback();
            }

            commandList.Reset();

            _availableCopyCommandLists.Enqueue(std::move(commandList));
        }
    });
}
