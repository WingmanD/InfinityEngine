#include "Engine.h"

#include "Rendering/Window.h"
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

    if (!_inputSubsystem.Initialize())
    {
        LOG(L"Failed to initialize Input Subsystem!");
        return false;
    }

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

    _assetManagerSubsystem.LoadAlwaysLoadedAssets();

    if (_renderingSubsystem->ConstructWindow(L"Infinity Engine") == nullptr)
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
        _renderingSubsystem->ForEachWindow([&msg](const Window* window)
        {
            while (PeekMessage(&msg, window->GetHandle(), 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            
            return true;
        });

        const double currentTime = GetTimeInSeconds();
        const double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        _inputSubsystem.Tick(deltaTime);
        // todo gameplay subsystem tick
        _renderingSubsystem->Tick(deltaTime);
    }

    Shutdown();
}

void Engine::Shutdown()
{
    _assetManagerSubsystem.Shutdown();
    _renderingSubsystem->Shutdown();
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

InputSubsystem& Engine::GetInputSubsystem()
{
    return _inputSubsystem;
}

AssetManager& Engine::GetAssetManager()
{
    return _assetManagerSubsystem;
}

RenderingSubsystem* Engine::GetRenderingSubsystem() const
{
    return _renderingSubsystem.get();
}
