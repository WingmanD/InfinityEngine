#pragma once

#include "PassKey.h"

class Engine;

// todo this can use CRTP to remove cost of virtual functions - we don't store subsystems in a vector or something
// the only thing that is different is rendering subsystem that uses it's own interface

class EngineSubsystem
{
public:
    explicit EngineSubsystem() = default;
    
    EngineSubsystem(const EngineSubsystem&) = delete;
    EngineSubsystem(EngineSubsystem&&) = default;
    EngineSubsystem& operator=(const EngineSubsystem&) = delete;
    EngineSubsystem& operator=(EngineSubsystem&&) = default;
    
    virtual ~EngineSubsystem() = default;

    bool CallInitialize(PassKey<Engine>);
    void CallTick(double deltaTime, PassKey<Engine>);
    void CallShutdown(PassKey<Engine>);
    
protected:
    virtual bool Initialize();
    virtual void Shutdown();
    virtual void Tick(double deltaTime);
};
