#include "DX12RenderingSubsystem.h"
#include "CompactSMInstancesComputeShader.h"
#include "Core.h"
#include "DX12GPUBuffer.h"
#include "DX12MaterialParameterRenderingData.h"
#include "DX12MaterialRenderingData.h"
#include "DX12RenderTarget.h"
#include "DX12Shader.h"
#include "DX12StaticMeshRenderingData.h"
#include "DX12TextWidgetRenderingProxy.h"
#include "DX12ViewportWidgetRenderingProxy.h"
#include "DX12WidgetRenderingProxy.h"
#include "DX12Window.h"
#include "DynamicGPUBufferUploader.h"
#include "SortComputeShader.h"
#include "ThreadPool.h"
#include "d3dx12/d3dx12.h"
#include "ECS/Components/CCamera.h"
#include "ECS/Systems/StaticMeshRenderingSystem.h"
#include "Engine/Engine.h"
#include "Rendering/InstanceBuffer.h"
#include "Rendering/Window.h"
#include "Rendering/Widgets/ViewportWidget.h"
#include <string>
#include <vector>

extern "C" {
__declspec(dllexport) extern const UINT D3D12SDKVersion = 613;
}

extern "C" {
__declspec(dllexport) extern const char* D3D12SDKPath = ".\\Libraries\\";
}

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
    DX12GraphicsCommandList* commandList,
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
        commandList->ResourceBarrier(1, &transition);
    }

    return buffer;
}

void DX12RenderingSubsystem::DrawScene(const ViewportWidget& viewport)
{
    if (_staticMeshRenderingSystems.IsEmpty())
    {
        return;
    }

    {
        const std::shared_ptr<Scene> scene = GetScene();
        CCamera* camera = viewport.GetCamera();
        const Transform& cameraTransform = camera->GetTransform(); // todo this triggers recalculation of matrices

        scene->ViewProjection = camera->GetViewProjectionMatrix().Transpose();
        scene->CameraLocationWS = cameraTransform.GetWorldLocation();
        scene->CameraForward = cameraTransform.GetForwardVector();
        scene->CameraUp = cameraTransform.GetUpVector();
        scene->HorizontalFOV = Math::ToRadians(camera->GetHorizontalFieldOfView() / 2.0f);
        scene->VerticalFOV = Math::ToRadians((camera->GetHorizontalFieldOfView() / 2.0f) / camera->GetAspectRatio());
        scene->DrawDistance = camera->GetFarClipPlane();
        scene->MarkAsDirty();
    }

    const std::shared_ptr<DX12Window> window = std::dynamic_pointer_cast<DX12Window>(viewport.GetParentWindow());

    DX12CommandList commandListStruct = window->RequestCommandList();
    DX12GraphicsCommandList* commandList = commandListStruct.CommandList.Get();

    // todo use multiple command lists, write to them in parallel after we get the results from the compute shader, if
    // the results require many draw calls

    for (StaticMeshRenderingSystem* renderingSystem : _staticMeshRenderingSystems)
    {
        // todo check if system's world is visible - we need to know which world the camera belongs to
        renderingSystem->GetInstanceBuffer().GetProxy<DynamicGPUBufferUploader<SMInstance>>()->Update(commandList);

        // todo multiple systems
        _cullingWorkGraph.SetInstanceBuffer(&renderingSystem->GetInstanceBuffer());
        _cullingWorkGraph.Dispatch(commandList, renderingSystem->GetInstanceBuffer().GetData(),
                                   renderingSystem->GetInstanceBuffer().Count());
        DX12Statics::TransitionUAV(*commandList, _cullingWorkGraph.GetVisibleInstances().GetBuffer().Get());
    }

    _cullingWorkGraph.GetVisibleInstances().ReadBackCounter(commandList);

    window->CloseCommandList(commandListStruct);

    window->ExecuteCommandLists();
    _commandQueue->Signal(_mainFence.Get(), ++_mainFenceValue);

    WaitForGPU(); // todo async, we need to pass a the viewport data and make RequestCommandList thread safe

    DX12CommandList commandListStructSort = window->RequestCommandList(viewport);
    DX12GraphicsCommandList* commandListSort = commandListStructSort.CommandList.Get();

    AppendStructuredBuffer<SMInstance>& visibleInstances = _cullingWorkGraph.GetVisibleInstances();

    DX12Statics::Transition(commandListSort, visibleInstances.GetBuffer().Get(), D3D12_RESOURCE_STATE_COMMON,
                            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    _sortCS->Run(*commandListSort, visibleInstances);
    _compactSMInstancesCS.lock()->Run(*commandListSort, visibleInstances);

    DX12Statics::Transition(commandListSort, visibleInstances.GetBuffer().Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                            D3D12_RESOURCE_STATE_COMMON);

    _cullingWorkGraph.ReadBackVisibleInstances(commandListSort);

    window->CloseCommandList(commandListStructSort);

    window->ExecuteCommandLists();
    _commandQueue->Signal(_mainFence.Get(), ++_mainFenceValue);

    WaitForGPU();

    DX12CommandList commandListStructDraw = window->RequestCommandList(viewport);
    DX12GraphicsCommandList* commandListDraw = commandListStructDraw.CommandList.Get();

    uint32 processedInstances = 0;
    const SMInstance* data = visibleInstances.GetReadBackData();

    DArray<SMInstance> visibleInstancesArray;
    visibleInstancesArray.Reserve(visibleInstances.Count());
    for (uint32 i = 0; i < visibleInstances.Count(); ++i)
    {
        visibleInstancesArray.Add(data[i]);
        LOG(L"{}: {} {} {} MI: {} Count: {}",
            i,
            data[i].World.m[0][3], data[i].World.m[1][3], data[i].World.m[2][3],
            data[i].MaterialIndex, data[i].Count);
    }

    while (processedInstances < visibleInstances.Count())
    {
        const SMInstance& firstInstance = data[processedInstances];

        const std::shared_ptr<StaticMesh> staticMesh = StaticMesh::GetMeshByID(firstInstance.MeshID);
        if (staticMesh != nullptr)
        {
            const std::shared_ptr<Material> material = Material::GetMaterialByID(firstInstance.MaterialID);
            if (material != nullptr)
            {
                const std::shared_ptr<DX12Shader> shader = material->GetShader<DX12Shader>();

                const DX12StaticMeshRenderingData* renderingData = staticMesh->GetRenderingData<
                    DX12StaticMeshRenderingData>();
                renderingData->SetupDrawing(commandListDraw, material);

                const DynamicGPUBuffer<MaterialParameter>& materialBuffer = _staticMeshRenderingSystems[0]->
                    GetMaterialParameterBuffer(firstInstance.MaterialID);
                materialBuffer.GetProxy<DynamicGPUBufferUploader<MaterialParameter>>()->Update(commandListDraw);

                shader->BindInstanceBuffers(*commandListDraw, visibleInstances, materialBuffer);

                commandListDraw->DrawIndexedInstanced(
                    static_cast<uint32>(staticMesh->GetIndices().size()),
                    firstInstance.Count,
                    0,
                    0,
                    processedInstances
                );
            }
            else
            {
                LOG(L"Failed to find material with ID: {}", firstInstance.MaterialID);
            }
        }
        else
        {
            LOG(L"Failed to find static mesh with ID: {}", firstInstance.MeshID);
        }

        processedInstances += firstInstance.Count;
    }

    window->CloseCommandList(commandListStructDraw);
    window->ExecuteCommandLists();
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

DX12Device* DX12RenderingSubsystem::GetDevice() const
{
    return _device.Get();
}

DXCompiler& DX12RenderingSubsystem::GetD3DCompiler() const
{
    return *_compiler.Get();
}

IDxcUtils& DX12RenderingSubsystem::GetDXCUtils() const
{
    return *_dxcUtils.Get();
}

ComPtr<ID3DBlob> DX12RenderingSubsystem::CompileDXILLibrary(const std::filesystem::path& libraryPath,
                                                            const std::wstring& target,
                                                            const DArray<DxcDefine>& defines /*= {}*/) const
{
    std::ifstream fileStream(libraryPath);
    if (!fileStream.is_open())
    {
        LOG(L"Failed to open shader library file: {}", libraryPath.wstring());
        return nullptr;
    }

    const std::string contents((std::istreambuf_iterator(fileStream)), std::istreambuf_iterator<char>());

    DXCompiler& compiler = GetD3DCompiler();
    IDxcUtils& dxcUtils = GetDXCUtils();

    ComPtr<IDxcBlobEncoding> source;
    dxcUtils.CreateBlob(contents.c_str(), static_cast<uint32>(contents.size()), DXC_CP_UTF8, source.GetAddressOf());

    DxcBuffer sourceBuffer;
    sourceBuffer.Ptr = source->GetBufferPointer();
    sourceBuffer.Size = source->GetBufferSize();
    sourceBuffer.Encoding = 0;

    ComPtr<IDxcIncludeHandler> includeHandler;
    dxcUtils.CreateDefaultIncludeHandler(&includeHandler);


    std::vector<LPCWSTR> arguments;
    arguments.push_back(L"-T");
    arguments.push_back(target.c_str());

    arguments.push_back(L"-I");

    const std::wstring includePath = libraryPath.parent_path().wstring();
    arguments.push_back(includePath.c_str());

    arguments.push_back(L"-Qstrip_debug");
    arguments.push_back(L"-Qstrip_reflect");

    arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);

    for (const DxcDefine& define : defines)
    {
        arguments.push_back(L"-D");
        arguments.push_back(define.Name);
        arguments.push_back(define.Value);
    }

    ComPtr<IDxcResult> compileResult;
    HRESULT result = compiler.Compile(
        &sourceBuffer,
        arguments.data(),
        static_cast<uint32>(arguments.size()),
        includeHandler.Get(),
        IID_PPV_ARGS(compileResult.GetAddressOf())
    );

    if (FAILED(result))
    {
        LOG(L"Failed to compile shader library {}!", libraryPath.stem().wstring());
        return nullptr;
    }

    ComPtr<ID3DBlob> libraryBlob;
    compileResult->GetStatus(&result);
    if (SUCCEEDED(result))
    {
        result = compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(libraryBlob.GetAddressOf()), nullptr);
        if (FAILED(result))
        {
            LOG(L"Failed to get library compilation result: {}", libraryPath.stem().wstring());
        }
    }

    CComPtr<IDxcBlobEncoding> errors;
    if (SUCCEEDED(compileResult->GetErrorBuffer(&errors)))
    {
        std::wstring errorString = Util::ToWString(static_cast<char*>(errors->GetBufferPointer()));
        if (!errorString.empty())
        {
            LOG(L"ERROR: {}", errorString);
        }
    }

    return libraryBlob;
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

uint32 DX12RenderingSubsystem::GetCBVSRVUAVDescriptorSize() const
{
    return _cbvSrvUavDescriptorSize;
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

#if DEBUG && !GPU_DEBUG
    {
        ComPtr<ID3D12Debug> debugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
    }
#endif
    ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&_dxgiFactory)));

    const HRESULT deviceResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&_device));
    if (FAILED(deviceResult))
    {
        LOG(L"Failed to create D3D12 device!");
        return false;
    }

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
    dsvHeapDesc.NumDescriptors = 16;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    _dsvHeap = DescriptorHeap(_device.Get(), dsvHeapDesc);

    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = 8192;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;
    _cbvHeap = std::make_shared<DescriptorHeap>(_device.Get(), cbvHeapDesc);

    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc;
    srvHeapDesc.NumDescriptors = 8192;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    srvHeapDesc.NodeMask = 0;
    _srvHeap = std::make_shared<DescriptorHeap>(_device.Get(), srvHeapDesc);

    DX12CommandList commandList;

    ThrowIfFailed(_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(commandList.CommandAllocator.GetAddressOf())));

    _graphicsMemory = new DirectX::DX12::GraphicsMemory(_device.Get());

    if (FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler))))
    {
        LOG(L"Failed to create DXC compiler!");
        return false;
    }

    if (FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_dxcUtils))))
    {
        LOG(L"Failed to create DXC utils!");
        return false;
    }

    _cullingWorkGraph.Initialize();
    _sortCS = AssetManager::Get().FindAssetByName<SortComputeShader>(Name(L"SortCS"));
    if (_sortCS == nullptr)
    {
        LOG(L"Failed to find SortCS asset! Play in editor will not work!");
    }
    else
    {
        _sortCS->Load();
    }

    const std::shared_ptr<CompactSMInstancesComputeShader> compactCS =
        AssetManager::Get().FindAssetByName<CompactSMInstancesComputeShader>(Name(L"CompactCS"));
    if (compactCS == nullptr)
    {
        LOG(L"Failed to find CompactCS asset! Play in editor will not work!");
    }
    else
    {
        compactCS->Load();
        _compactSMInstancesCS = compactCS;
    }

    return true;
}

void DX12RenderingSubsystem::Shutdown()
{
    RenderingSubsystem::Shutdown();
}

void DX12RenderingSubsystem::Tick(double deltaTime)
{
    RenderingSubsystem::Tick(deltaTime);

    GetEventQueue().ProcessEvents();

    for (const std::shared_ptr<DX12Window>& window : _windows)
    {
        window->Tick(deltaTime);
    }

    HandleCopyLists();

    for (const std::shared_ptr<DX12Window>& window : _windows)
    {
        // the reason we can't use the thread pool currently is the command list request/close that is not multi-threaded, also, command lists must be executed in order of windows
        // GetEngine()->GetThreadPool().EnqueueTask([window, weakThis]()
        // {
        //     
        // });

        window->Render({});

        ++_mainFenceValue;
        ThrowIfFailed(_commandQueue->Signal(_mainFence.Get(), _mainFenceValue));

        WaitForGPU();

        window->Present({});
        window->EndFrame({});
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

std::unique_ptr<WidgetRenderingProxy> DX12RenderingSubsystem::CreateViewportWidgetRenderingProxy()
{
    return std::make_unique<DX12ViewportWidgetRenderingProxy>();
}

void DX12RenderingSubsystem::RegisterStaticMeshRenderingSystem(StaticMeshRenderingSystem* system)
{
    system->GetInstanceBuffer().SetProxy(std::make_unique<DynamicGPUBufferUploader<SMInstance>>());

    _staticMeshRenderingSystems.Add(system);
}

void DX12RenderingSubsystem::UnregisterStaticMeshRenderingSystem(StaticMeshRenderingSystem* system)
{
    _staticMeshRenderingSystems.Remove(system);
}

void DX12RenderingSubsystem::InitializeMaterialInstanceBuffer(DynamicGPUBuffer<MaterialParameter>& instanceBuffer,
                                                              Type* type)
{
    instanceBuffer.SetProxy(std::make_unique<DynamicGPUBufferUploader<MaterialParameter>>(type));
}

std::unique_ptr<GPUBuffer> DX12RenderingSubsystem::CreateBuffer(GPUBuffer::EType type)
{
    switch (type)
    {
    case GPUBuffer::EType::Default:
        return std::make_unique<DX12GPUBuffer>();
    default:
        DEBUG_BREAK();
        return nullptr;
    }
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
