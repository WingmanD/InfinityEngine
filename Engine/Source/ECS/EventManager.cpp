#include "EventManager.h"
#include "Event.h"

void EventManager::RegisterEvent(EventBase& event, PassKey<EventBase>)
{
    _events.Add(&event);
}

void EventManager::UnregisterEvent(EventBase& event, PassKey<EventBase>)
{
    _events.Remove(&event);
}

void EventManager::UpdateQueries(const EntityListGraph& entityListGraph)
{
    // todo we should cache information about which events need update
    for (EventBase* event : _events)
    {
        event->UpdateQuery(entityListGraph);
    }
}
