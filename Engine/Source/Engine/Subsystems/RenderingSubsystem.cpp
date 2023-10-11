#include "RenderingSubsystem.h"

RenderingSubsystem::RenderingSubsystem() : _eventQueue(this)
{
}

EventQueue<RenderingSubsystem>& RenderingSubsystem::GetEventQueue()
{
    return _eventQueue;
}
