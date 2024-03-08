#include "EngineSubsystem.h"

bool EngineSubsystem::CallInitialize(PassKey<Engine>)
{
    return Initialize();
}

void EngineSubsystem::CallTick(double deltaTime, PassKey<Engine>)
{
    Tick(deltaTime);
}

void EngineSubsystem::CallShutdown(PassKey<Engine>)
{
    Shutdown();
}

bool EngineSubsystem::Initialize()
{
    return true;
}

void EngineSubsystem::Shutdown()
{
}

void EngineSubsystem::Tick(double deltaTime)
{
}
