#pragma once

#include "Core.h"
#include "ThreadPool.h"
#include "Subsystems/AssetManager.h"
#include "Subsystems/RenderingSubsystem.h"

class Engine
{
public:
    explicit Engine();
    
    static Engine& Get();

    bool Initialize(HINSTANCE hInstance);

    void Run();

    void RequestExit();

    HINSTANCE GetHandle() const;

    ThreadPool& GetThreadPool();

    AssetManager& GetAssetManager();
    RenderingSubsystem* GetRenderingSubsystem() const;

private:
    static Engine _mainInstance;
    
    HINSTANCE _hInstance = nullptr;

    bool _exitRequested = false;

    ThreadPool _threadPool;

    AssetManager _assetManagerSubsystem;
    std::unique_ptr<RenderingSubsystem> _renderingSubsystem;

private:
    void Shutdown();
};
