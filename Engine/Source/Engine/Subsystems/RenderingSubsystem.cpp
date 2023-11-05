#include "RenderingSubsystem.h"
#include "Engine/Engine.h"

RenderingSubsystem::RenderingSubsystem() : _eventQueue(this)
{
}

RenderingSubsystem& RenderingSubsystem::Get()
{
    return *Engine::Get().GetRenderingSubsystem();
}

EventQueue<RenderingSubsystem>& RenderingSubsystem::GetEventQueue()
{
    return _eventQueue;
}
