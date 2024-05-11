#include "Event.h"
#include "EventManager.h"

EventBase::~EventBase()
{
    _eventManager->UnregisterEvent(*this, {});
}

void EventBase::SetEventManager(EventManager& eventManager)
{
    _eventManager = &eventManager;
    _eventManager->RegisterEvent(*this, {});
}

const Archetype& EventBase::GetArchetype() const
{
    return _archetype;
}

void EventBase::SetArchetype(const Archetype& archetype)
{
    _archetype = archetype;
}

const Archetype& EventBase::GetArchetype()
{
    return _archetype;
}
