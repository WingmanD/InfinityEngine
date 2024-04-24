#include "DX12Window.h"
#include "DX12RenderingSubsystem.h"
#include "DX12WidgetRenderingProxy.h"
#include "DX12ViewportWidgetRenderingProxy.h"
#include <DirectXColors.h>

#include "Rendering/Widgets/ViewportWidget.h"

DX12Window::DX12Window(uint32 width, uint32 height, const std::wstring& title) :
    Window(width, height, title)
{
}

bool DX12Window::Initialize()
{
    if (!Window::Initialize())
    {
        return false;
    }

    _swapChain.Reset();

    const DX12RenderingSubsystem& renderingSubsystem = GetDX12RenderingSubsystem();

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swapChainDesc.BufferDesc.Width = GetWidth();
    swapChainDesc.BufferDesc.Height = GetHeight();
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = renderingSubsystem.GetFrameBufferFormat();
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SampleDesc.Count = renderingSubsystem.GetMSAASampleCount();
    swapChainDesc.SampleDesc.Quality = renderingSubsystem.GetMSAAQuality() - 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = renderingSubsystem.GetBufferCount();
    swapChainDesc.OutputWindow = GetHandle();
    swapChainDesc.Windowed = true;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ThrowIfFailed(renderingSubsystem.GetDXGIFactory()->CreateSwapChain(
        renderingSubsystem.GetCommandQueue(),
        &swapChainDesc,
        _swapChain.GetAddressOf()));

    ThrowIfFailed(renderingSubsystem.GetDXGIFactory()->MakeWindowAssociation(GetHandle(), 0));

    RequestResize(GetWidth(), GetHeight());

    return true;
}

void DX12Window::Render(PassKey<DX12RenderingSubsystem>)
{
    const WindowState state = GetState();
    if (state == WindowState::Minimized || state == WindowState::BeingResized)
    {
        return;
    }

    DX12CommandList dx12CommandList = RequestUninitializedCommandList();
    DX12GraphicsCommandList* commandList = dx12CommandList.CommandList.Get();

    const PendingResize& pendingResize = GetPendingResize();
    if (pendingResize.IsValid)
    {
        ResizeImplementation(commandList);
    }

    const FrameBuffer& frameBuffer = _frameBuffers[_currentFrameBufferIndex];

    const CD3DX12_RESOURCE_BARRIER transitionPresentToTarget = CD3DX12_RESOURCE_BARRIER::Transition(
        frameBuffer.RenderTargetView.Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList->ResourceBarrier(1, &transitionPresentToTarget);

    commandList->RSSetViewports(1, &_fullWindowViewport);
    commandList->RSSetScissorRects(1, &_fullWindowRect);

    commandList->ClearRenderTargetView(frameBuffer.RTVDescriptorHandle, DirectX::Colors::Black, 0, nullptr);
    commandList->ClearDepthStencilView(_depthStencilView,
                                       D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                                       1.0f,
                                       0,
                                       0,
                                       nullptr);
    commandList->OMSetRenderTargets(1, &frameBuffer.RTVDescriptorHandle, true, &_depthStencilView);
    
    for (const std::shared_ptr<Layer>& layer : GetLayers())
    {
        if (layer->RootWidget == nullptr)
        {
            continue;
        }
        
        commandList->ClearDepthStencilView(_depthStencilView,
                                           D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                                           1.0f,
                                           0,
                                           0,
                                           nullptr);
        dx12CommandList = layer->RootWidget->GetRenderingProxy<DX12WidgetRenderingProxy>().SetupDrawing(dx12CommandList);
        commandList = dx12CommandList.CommandList.Get();
    
    }
    
    CloseCommandList(dx12CommandList);

    ExecuteCommandLists();
}

void DX12Window::Present(PassKey<DX12RenderingSubsystem>)
{
    _swapChain->Present(0, DXGI_PRESENT_DO_NOT_WAIT);

    _currentFrameBufferIndex = (_currentFrameBufferIndex + 1) % static_cast<int32>(_frameBuffers.size());
}

void DX12Window::EndFrame(PassKey<DX12RenderingSubsystem>)
{
    while (!_closedCommandLists.IsEmpty())
    {
        DX12CommandList& closedCommandList = _closedCommandLists.Back();
        closedCommandList.Reset();

        _availableCommandLists.Add(closedCommandList);
        _closedCommandLists.PopBack();
    }
}

const D3D12_VIEWPORT& DX12Window::GetViewport() const
{
    return _fullWindowViewport;
}

DX12CommandList DX12Window::RequestCommandList()
{
    DX12CommandList commandList = RequestCommandListImplementation();
    
    commandList.CommandList->RSSetViewports(1, &_fullWindowViewport);
    commandList.CommandList->RSSetScissorRects(1, &_fullWindowRect);

    return commandList;
}

DX12CommandList DX12Window::RequestCommandList(const ViewportWidget& viewport)
{
    DX12CommandList commandList = RequestCommandListImplementation();

    const DX12ViewportWidgetRenderingProxy& renderingProxy = viewport.GetRenderingProxy<DX12ViewportWidgetRenderingProxy>();

    commandList.CommandList->RSSetViewports(1, &renderingProxy.GetViewport());
    commandList.CommandList->RSSetScissorRects(1, &viewport.GetRect());

    commandList.CommandList->ClearRenderTargetView(_frameBuffers[_currentFrameBufferIndex].RTVDescriptorHandle,
        DirectX::Colors::Black,
        1,
        &viewport.GetRect());
    commandList.CommandList->ClearDepthStencilView(_depthStencilView,
                                       D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                                       1.0f,
                                       0,
                                       1,
                                       &viewport.GetRect());

    return commandList;
}

void DX12Window::CloseCommandList(DX12CommandList& commandList)
{
    const CD3DX12_RESOURCE_BARRIER transitionTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
    _frameBuffers[_currentFrameBufferIndex].RenderTargetView.Get(),
    D3D12_RESOURCE_STATE_RENDER_TARGET,
    D3D12_RESOURCE_STATE_PRESENT);

    commandList.CommandList->ResourceBarrier(1, &transitionTargetToPresent);
    
    commandList.Close();
    _closedCommandLists.Add(commandList);
}

void DX12Window::ExecuteCommandLists()
{
    static std::vector<ID3D12CommandList*> commandLists;
    commandLists.clear();

    for (DX12CommandList& commandList : _closedCommandLists)
    {
        if (commandList.State != DX12CommandList::ECommandListState::Closed)
        {
            continue;
        }

        commandLists.push_back(commandList.CommandList.Get());
        commandList.State = DX12CommandList::ECommandListState::Executing;
    }

    const DX12RenderingSubsystem& renderingSubsystem = GetDX12RenderingSubsystem();
    renderingSubsystem.GetCommandQueue()->ExecuteCommandLists(
        static_cast<uint32>(commandLists.size()),
        commandLists.data());
}

DX12RenderingSubsystem& DX12Window::GetDX12RenderingSubsystem() const
{
    return dynamic_cast<DX12RenderingSubsystem&>(RenderingSubsystem::Get());
}

void DX12Window::ResizeImplementation(DX12GraphicsCommandList* commandList)
{
    DX12RenderingSubsystem& renderingSubsystem = GetDX12RenderingSubsystem();
    DX12Device* device = renderingSubsystem.GetDevice();

    PendingResize& pendingResize = GetPendingResize();
    pendingResize.IsValid = false;
    const uint32 width = pendingResize.Width;
    const uint32 height = pendingResize.Height;

    const uint32 bufferCount = renderingSubsystem.GetBufferCount();

    for (FrameBuffer& frameBuffer : _frameBuffers)
    {
        if (frameBuffer.RTVDescriptorHandle.ptr != 0)
        {
            renderingSubsystem.GetRTVHeap().FreeHeapResourceHandle(frameBuffer.RTVDescriptorHandle);
        }
        frameBuffer.RenderTargetView.Reset();
    }

    ThrowIfFailed(_swapChain->ResizeBuffers(
        bufferCount,
        width,
        height,
        renderingSubsystem.GetFrameBufferFormat(),
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

    _currentFrameBufferIndex = 0;

    if (_frameBuffers.size() != bufferCount)
    {
        _frameBuffers.clear();

        for (auto i = 0u; i < bufferCount; ++i)
        {
            _frameBuffers.emplace_back();
        }
    }

    for (int32 i = 0; i < _frameBuffers.size(); ++i)
    {
        const D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle = renderingSubsystem.GetRTVHeap().RequestHeapResourceHandle();

        ThrowIfFailed(_swapChain->GetBuffer(i, IID_PPV_ARGS(&_frameBuffers[i].RenderTargetView)));
        device->CreateRenderTargetView(_frameBuffers[i].RenderTargetView.Get(), nullptr, rtvHeapHandle);

        _frameBuffers[i].RTVDescriptorHandle = rtvHeapHandle;

        std::wstring name = GetTitle() + std::wstring(L": Frame Buffer ") + std::to_wstring(i);
        _frameBuffers[i].RenderTargetView->SetName(name.c_str());
    }

    _depthStencilBuffer.Reset();

    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = width;
    depthStencilDesc.Height = height;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = renderingSubsystem.GetDepthStencilFormat();
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;

    const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);

    if (_depthStencilView.ptr != 0)
    {
        _depthStencilBuffer.Reset();
        renderingSubsystem.GetDSVHeap().FreeHeapResourceHandle(_depthStencilView);
    }

    ThrowIfFailed(device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(_depthStencilBuffer.GetAddressOf())));

    _depthStencilView = renderingSubsystem.GetDSVHeap().RequestHeapResourceHandle();
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = renderingSubsystem.GetDepthStencilFormat();
    dsvDesc.Texture2D.MipSlice = 0;
    device->CreateDepthStencilView(_depthStencilBuffer.Get(), &dsvDesc, _depthStencilView);

    const auto transitionDSV = CD3DX12_RESOURCE_BARRIER::Transition(
        _depthStencilBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_DEPTH_WRITE);
    commandList->ResourceBarrier(1, &transitionDSV);

    _fullWindowViewport.TopLeftX = 0;
    _fullWindowViewport.TopLeftY = 0;
    _fullWindowViewport.Width = static_cast<float>(width);
    _fullWindowViewport.Height = static_cast<float>(height);
    _fullWindowViewport.MinDepth = 0.0f;
    _fullWindowViewport.MaxDepth = 1.0f;

    OnViewportChanged.Broadcast(_fullWindowViewport);

    _fullWindowRect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};

    OnResized();
}

DX12CommandList DX12Window::RequestCommandListImplementation()
{
    DX12CommandList commandList = RequestUninitializedCommandList();
    
    const FrameBuffer& frameBuffer = _frameBuffers[_currentFrameBufferIndex];
    
    const CD3DX12_RESOURCE_BARRIER transitionPresentToTarget = CD3DX12_RESOURCE_BARRIER::Transition(
    frameBuffer.RenderTargetView.Get(),
    D3D12_RESOURCE_STATE_PRESENT,
    D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList.CommandList->ResourceBarrier(1, &transitionPresentToTarget);
    
    commandList.CommandList->OMSetRenderTargets(
        1,
        &frameBuffer.RTVDescriptorHandle,
        true,
        &_depthStencilView);

    return commandList;
}

DX12CommandList DX12Window::RequestUninitializedCommandList()
{
    DX12CommandList commandList;

    const DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();
    DX12Device* device = renderingSubsystem.GetDevice();

    if (_availableCommandLists.IsEmpty())
    {
        ThrowIfFailed(device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(commandList.CommandAllocator.GetAddressOf())));

        ThrowIfFailed(device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            commandList.CommandAllocator.Get(),
            nullptr,
            IID_PPV_ARGS(commandList.CommandList.GetAddressOf())));
    }
    else
    {
        commandList = _availableCommandLists.Back();
        _availableCommandLists.PopBack();
    }

    ID3D12DescriptorHeap* descriptorHeaps[] = {DX12RenderingSubsystem::Get().GetCBVHeap()->GetHeap().Get()};
    commandList.CommandList->SetDescriptorHeaps(1, descriptorHeaps);

    return commandList;
}
