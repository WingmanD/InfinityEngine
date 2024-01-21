#include "DX12Window.h"
#include "Engine/Engine.h"
#include "DX12RenderingSubsystem.h"
#include "DX12RenderTarget.h"
#include "DX12Shader.h"
#include "DX12StaticMeshRenderingData.h"
#include "DX12WidgetRenderingProxy.h"
#include "Rendering/StaticMeshRenderingData.h"
#include "MaterialParameterTypes.h"
#include <DirectXColors.h>
#include <numbers>

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

    AssetManager& assetManager = Engine::Get().GetAssetManager();
    _staticMeshTest = assetManager.FindAssetByName<StaticMesh>(L"SwarmDrone");
    if (_staticMeshTest != nullptr)
    {
        _staticMeshTest->Load();
    }

    return true;
}

void DX12Window::Render(PassKey<DX12RenderingSubsystem>)
{
    const WindowState state = GetState();
    if (state == WindowState::Minimized || state == WindowState::BeingResized)
    {
        return;
    }

    DX12RenderingSubsystem& renderingSubsystem = GetDX12RenderingSubsystem();
    DX12CommandList dx12CommandList = renderingSubsystem.RequestCommandList();
    ID3D12GraphicsCommandList* commandList = dx12CommandList.CommandList.Get();

    const PendingResize& pendingResize = GetPendingResize();
    if (pendingResize.IsValid)
    {
        ResizeImplementation(commandList);
    }

    FrameBuffer& frameBuffer = _frameBuffers[_currentFrameBufferIndex];

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

    {
        if (_staticMeshTest != nullptr)
        {
            using namespace DirectX;

            Vector3 pos = Vector3(5.0f, 1.0f, 2.0f);
            Vector3 target = Vector3(0.0f, 0.0f, 0.0f);
            Vector3 up = Vector3(0.0f, 0.0f, 1.0f);

            Matrix view = XMMatrixLookAtLH(pos, target, up);
            Matrix world = Matrix::CreateTranslation(0.0f, 0.0f, 0.0f);
            const float aspectRatio = static_cast<float>(GetWidth()) / static_cast<float>(GetHeight());
            Matrix proj = XMMatrixPerspectiveFovLH(0.25f * static_cast<float>(std::numbers::pi), aspectRatio, 1.0f,
                                                   1000.0f);

            Material* material = _staticMeshTest->GetMaterial().get();

            PerPassConstants* perPassConstants = material->GetParameter<PerPassConstants>("GPerPassConstants");
            perPassConstants->Time += 0.008f;
            XMStoreFloat4x4(&perPassConstants->World, XMMatrixTranspose(world));
            XMStoreFloat4x4(&perPassConstants->ViewProjection, XMMatrixTranspose(view * proj));
            perPassConstants->CameraPosition = pos;
            perPassConstants->CameraDirection = target - pos;
            perPassConstants->CameraDirection.Normalize();

            if (_staticMeshTest->GetRenderingData()->IsUploaded())
            {
                static_cast<DX12StaticMeshRenderingData*>(_staticMeshTest->GetRenderingData())->SetupDrawing(
                    commandList, _staticMeshTest->GetMaterial());
            }
        }
    }

    // when we come to a viewport, we need to give it this command list, and it will take more command lists, after that,
    // we must request a new command list to render other UI elements to maintain order - UI elements are overlayed
    // todo think about layers - in that case, if current layer has a viewport, the next layer must have a new command list
    // note that even like this, we can record viewport commands and the whole UI commands in parallel, but we must maintain order of releasing
    // command lists
    for (const std::shared_ptr<Layer>& layer : GetLayers())
    {
        if (layer->RootWidget != nullptr)
        {
            commandList->ClearDepthStencilView(_depthStencilView,
                                               D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                                               1.0f,
                                               0,
                                               0,
                                               nullptr);
            dynamic_cast<DX12WidgetRenderingProxy&>(layer->RootWidget->GetRenderingProxy()).SetupDrawing(commandList);
        }
    }

    const CD3DX12_RESOURCE_BARRIER transitionTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
        frameBuffer.RenderTargetView.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);

    // todo this must be in a new command list, as it must be the last thing we do
    commandList->ResourceBarrier(1, &transitionTargetToPresent);
    renderingSubsystem.CloseCommandList(dx12CommandList);
}

void DX12Window::Present(PassKey<DX12RenderingSubsystem>)
{
    _swapChain->Present(0, DXGI_PRESENT_DO_NOT_WAIT);

    _currentFrameBufferIndex = (_currentFrameBufferIndex + 1) % static_cast<int32>(_frameBuffers.size());
}

const D3D12_VIEWPORT& DX12Window::GetViewport() const
{
    return _fullWindowViewport;
}

DX12RenderingSubsystem& DX12Window::GetDX12RenderingSubsystem() const
{
    return dynamic_cast<DX12RenderingSubsystem&>(RenderingSubsystem::Get());
}

void DX12Window::ResizeImplementation(ID3D12GraphicsCommandList* commandList)
{
    DX12RenderingSubsystem& renderingSubsystem = GetDX12RenderingSubsystem();
    ID3D12Device* device = renderingSubsystem.GetDevice();

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
