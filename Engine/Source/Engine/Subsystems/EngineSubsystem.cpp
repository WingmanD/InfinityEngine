#include "EngineSubsystem.h"

bool EngineSubsystem::CallInitialize(PassKey<Engine>)
{
    return Initialize() && PostInitialize();
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

bool EngineSubsystem::PostInitialize()
{
    return true;
}

void EngineSubsystem::Shutdown()
{
}

void EngineSubsystem::Tick(double deltaTime)
{
}
