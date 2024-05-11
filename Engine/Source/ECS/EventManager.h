#pragma once

#include "PassKey.h"
#include "Containers/DArray.h"

class EntityListGraph;
class EventBase;

class EventManager
{
public:
    void RegisterEvent(EventBase& event, PassKey<EventBase>);
    void UnregisterEvent(EventBase& event, PassKey<EventBase>);

    void UpdateQueries(const EntityListGraph& entityListGraph);

private:
    DArray<EventBase*> _events;
};
