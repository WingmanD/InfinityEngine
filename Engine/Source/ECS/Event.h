#pragma once

#include "TypeSet.h"
#include "Containers/DArray.h"
#include "Containers/LockFreeQueue.h"
#include "ECS/Archetype.h"
#include "ECS/EntityListGraph.h"

class SystemBase;
class EventManager;
class Archetype;
class Entity;
class EntityList;
class EventDispatcherBase;

template <typename ComponentList, typename SystemType>
concept IsCompatible = ComponentList::template ContainsAll<typename SystemType::RequiredComponents>();

class EventBase
{
public:
    explicit EventBase() = default;

    EventBase(const EventBase&) = delete;
    EventBase(EventBase&&) = delete;

    EventBase& operator=(const EventBase&) = delete;
    EventBase& operator=(EventBase&&) = delete;

    virtual ~EventBase();

    virtual void UpdateQuery(const EntityListGraph& entityListGraph) = 0;

    void SetEventManager(EventManager& eventManager);
    const Archetype& GetArchetype() const;

protected:
    void SetArchetype(const Archetype& archetype);
    const Archetype& GetArchetype();

private:
    EventManager* _eventManager = nullptr;
    Archetype _archetype;
};

template <typename ComponentList, typename... Args> requires IsA<ComponentList, TypeSetBase>
class Event : public EventBase
{
public:
    friend EventDispatcherBase;

    struct EventData
    {
        Entity* Entity;
        std::tuple<Args...> Arguments;
    };

    struct EntityListStruct
    {
        Archetype EntityArchetype;
        LockFreeQueue<EventData> Queue;
    };

public:
    Event()
    {
        SetArchetype(Archetype::Create<ComponentList>());
    }

public:
    virtual void UpdateQuery(const EntityListGraph& entityListGraph) override
    {
        ECSQuery query;
        entityListGraph.Query(query, GetArchetype());

        _archetypeToEntityListIndex.clear();
        _entityListsArray.Clear();

        for (const EntityList* entityList : query.GetEntityLists())
        {
            Archetype archetype = entityList->GetArchetype();
            _archetypeToEntityListIndex[archetype.GetID()] = _entityListsArray.Count();

            _entityListsArray.Add({archetype, {}});
        }
    }

    template <typename SystemType> requires IsA<SystemType, SystemBase> && IsCompatible<ComponentList, SystemType>
    void Add(Entity& entity, const Archetype& archetype, Args... args, PassKey<SystemType>)
    {
        Add(entity, archetype, args..., {});
    }

    void Add(Entity& entity, const Archetype& archetype, Args... args, PassKey<World>)
    {
        Add(entity, archetype, args..., {});
    }

    DArray<EntityListStruct>& GetEntityLists()
    {
        return _entityListsArray;
    }

private:
    std::unordered_map<uint64, uint64> _archetypeToEntityListIndex;
    DArray<EntityListStruct> _entityListsArray;

private:
    EntityListStruct& GetEntityList(const Archetype& archetype)
    {
        const uint64 index = _archetypeToEntityListIndex[archetype.GetID()];
        return _entityListsArray[index];
    }

    void Add(Entity& entity, const Archetype& archetype, Args... args)
    {
        EntityListStruct& entityList = GetEntityList(archetype);
        EventData eventData = {&entity, std::forward_as_tuple(args...)};
        entityList.Queue.Enqueue(std::move(eventData));
    }
};

struct EventHandle
{
public:
    uint64 ID = 0;

public:
    bool IsValid() const
    {
        return ID != 0;
    }
};

class EventDispatcherBase
{
protected:
    template <typename ComponentList, typename... Args> requires IsA<ComponentList, TypeSetBase>
    void SignalEvent(Event<ComponentList, Args...>& event, Entity& entity, const Archetype& archetype, Args... args)
    {
        event.Add(entity, archetype, args...);
    }
};

template <typename ComponentList, typename... Args> requires IsA<ComponentList, TypeSetBase>
class EventDispatcher : public EventDispatcherBase
{
public:
    [[nodiscard]] EventHandle RegisterListener(Event<ComponentList, Args...>& event)
    {
        const uint64 id = _idGenerator.GenerateID();
        _listener.Add({&event, id});
        _handleIDToIndex[id] = _listener.Count() - 1;

        return {id};
    }

    void UnregisterListener(const EventHandle& handle)
    {
        const auto it = _handleIDToIndex.find(handle.ID);
        if (it == _handleIDToIndex.end())
        {
            return;
        }

        size_t index = it->second;
        _handleIDToIndex.erase(handle.ID);

        if (index == _listener.Count() - 1)
        {
            _listener.PopBack();
            return;
        }

        _listener.RemoveAtSwap(index);

        _handleIDToIndex[_listener[index].ID] = index;
    }

    template <typename SystemType> requires IsA<SystemType, SystemBase> && IsCompatible<ComponentList, SystemType>
    void Add(Entity& entity, const Archetype& archetype, Args... args, PassKey<SystemType>)
    {
        Add(entity, archetype, args...);
    }

    void Add(Entity& entity, const Archetype& archetype, Args... args, PassKey<World>)
    {
        Add(entity, archetype, args...);
    }

private:
    struct Listener
    {
        Event<ComponentList, Args...>* Event;
        uint64 ID;
    };

    DArray<Listener> _listener;

    std::map<uint64, size_t> _handleIDToIndex;
    IDGenerator<uint64> _idGenerator;

private:
    void Add(Entity& entity, const Archetype& archetype, Args... args)
    {
        for (const Listener& listener : _listener)
        {
            SignalEvent(*listener.Event, entity, archetype, args...);
        }
    }
};
