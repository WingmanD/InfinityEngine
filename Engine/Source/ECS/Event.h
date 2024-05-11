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
    struct EventData
    {
        Entity* Entity;
        std::tuple<Args...> Arguments;
    };

    struct EntityListStruct
    {
        Archetype EntityArchetype;
        LockFreeQueue<EventData> EntityListInput;
        DArray<EventData> EntityListOutput;

        void Update()
        {
            EventData eventData;
            while (EntityListInput.Dequeue(eventData))
            {
                EntityListOutput.Add(eventData);
            }
        }
    };

public:
    Event()
    {
        SetArchetype(Archetype::Create<ComponentList>());
    }

public:
    virtual void UpdateQuery(const EntityListGraph& entityListGraph)
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
    void Add(Entity* entity, const Archetype& archetype, Args... args, PassKey<SystemType>)
    {
        EntityListStruct& entityList = GetEntityList(archetype);
        EventData eventData = {entity, std::forward_as_tuple(args...)};
        entityList.EntityListInput.Enqueue(std::move(eventData));
    }

    DArray<EntityListStruct>& GetEntityLists()
    {
        return _entityListsArray;
    }

    void Clear()
    {
        EventData eventData; 
        for (EntityListStruct& entityList : _entityListsArray)
        {
            while (entityList.EntityListInput.Dequeue(eventData));
        }
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
};
