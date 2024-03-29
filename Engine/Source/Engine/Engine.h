﻿#pragma once

#include "Core.h"
#include "ThreadPool.h"
#include "Subsystems/AssetManager.h"
#include "Subsystems/GameplaySubsystem.h"
#include "Subsystems/InputSubsystem.h"
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
    ThreadPool& GetWaitThreadPool();

    EventQueue<Engine>& GetMainEventQueue();

    InputSubsystem& GetInputSubsystem();
    AssetManager& GetAssetManager();
    GameplaySubsystem& GetGameplaySubsystem();
    RenderingSubsystem* GetRenderingSubsystem() const;

private:
    static Engine _mainInstance;
    
    HINSTANCE _hInstance = nullptr;

    bool _exitRequested = false;

    ThreadPool _threadPool;
    ThreadPool _waitThreadPool{100};

    EventQueue<Engine> _eventQueue;

    InputSubsystem _inputSubsystem{};
    AssetManager _assetManagerSubsystem;
    GameplaySubsystem _gameplaySubsystem;
    std::unique_ptr<RenderingSubsystem> _renderingSubsystem;

private:
    void Shutdown();
};
