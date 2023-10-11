#include "Engine.h"
#include "Rendering/DX12/DX12RenderingSubsystem.h"

Engine Engine::_mainInstance;

Engine::Engine()
{
    _renderingSubsystem = std::make_unique<DX12RenderingSubsystem>();
}

Engine& Engine::Get()
{
    return _mainInstance;
}

bool Engine::Initialize(HINSTANCE hInstance)
{
    _hInstance = hInstance;
    
    if (!_assetManagerSubsystem.Initialize())
    {
        LOG(L"Failed to initialize Asset Manager Subsystem!");
        return false;
    }

    if (!_renderingSubsystem->Initialize())
    {
        LOG(L"Failed to initialize Rendering Subsystem!");
        return false;
    }

    if (_renderingSubsystem->ConstructWindow(L"Swarm Engine") == nullptr)
    {
        LOG(L"Failed to construct main window!");
        return false;
    }

    return true;
}

void Engine::Run()
{
    MSG msg = {nullptr, 0};

    static double lastTime = GetTimeInSeconds();
    while (!_exitRequested)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        const double currentTime = GetTimeInSeconds();
        const double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // todo gameplay subsystem tick
        _renderingSubsystem->Tick(deltaTime);
    }

    Shutdown();
}

void Engine::Shutdown()
{
    TRACE_LOG("Engine shut down");
}

void Engine::RequestExit()
{
    _exitRequested = true;
}

HINSTANCE Engine::GetHandle() const
{
    return _hInstance;
}

ThreadPool& Engine::GetThreadPool()
{
    return _threadPool;
}

AssetManager& Engine::GetAssetManager()
{
    return _assetManagerSubsystem;
}

RenderingSubsystem* Engine::GetRenderingSubsystem() const
{
    return _renderingSubsystem.get();
}
