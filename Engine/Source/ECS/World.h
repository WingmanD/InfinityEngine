#pragma once

#include "BoundingBox.h"
#include "Event.h"
#include "EventManager.h"
#include "Containers/EventQueue.h"
#include "Containers/ObjectTypeMap.h"
#include "ECS/DirtyTracker.h"
#include "ECS/EntityListGraph.h"
#include "ECS/SystemScheduler.h"
#include "ECS/World.reflection.h"
#include "ECS/Components/Component.h"

class EntityTemplate;
class GameplaySubsystem;
class Type;
class Entity;
class CTransform;

REFLECTED()
class World : public Object
{
    GENERATED()

public:
    PROPERTY()
    EventDispatcher<TypeSet<CTransform>> OnTransformChanged;

    static BoundingBox WorldBounds;

    template <typename ComponentType> requires IsA<ComponentType, Component>
    struct AddComponentResult
    {
        SharedObjectPtr<ComponentType> Component;
        Entity* NewEntity = nullptr;
    };

public:
    explicit World();

    World(const World& other);

    void CreateEntityAsync(const Archetype& archetype);
    void CreateEntityAsync(const Archetype& archetype, uint32 count);

    template <typename Func>
    void CreateEntityAsync(const Archetype& archetype, Func onCreated)
    {
        _eventQueue.Enqueue([archetype, onCreated](World* world)
        {
            Entity& newEntity = world->CreateEntity(archetype);
            onCreated(newEntity);
        });
    }

    template <typename Func>
    void CreateEntityAsync(const Archetype& archetype, uint32 count, Func onCreated)
    {
        _eventQueue.Enqueue([archetype, onCreated, count](World* world)
        {
            for (uint32 i = 0; i < count; ++i)
            {
                Entity& newEntity = world->CreateEntity(archetype);
                onCreated(newEntity);
            }
        });
    }

    void DestroyEntityAsync(Entity& entity);

    template <typename Func>
    void DestroyEntityAsync(Entity& entity, Func onDestroyed)
    {
        _eventQueue.Enqueue([&entity, onDestroyed](World* world)
        {
            world->DestroyEntity(entity);
            onDestroyed();
        });
    }

    Entity& CreateEntity(const std::shared_ptr<EntityTemplate>& entityTemplate);
    void CreateEntity(const std::shared_ptr<EntityTemplate>& entityTemplate, uint32 count);

    template <typename Func>
    void CreateEntityAsync(const std::shared_ptr<EntityTemplate>& entityTemplate, Func onCreated)
    {
        _eventQueue.Enqueue([entityTemplate, onCreated](World* world)
        {
            Entity& newEntity = world->CreateEntity(entityTemplate);
            onCreated(newEntity);
        });
    }

    template <typename FuncInit, typename FuncOnCreated>
    void CreateEntityAsync(const std::shared_ptr<EntityTemplate>& entityTemplate, FuncInit preInitialize, FuncOnCreated onCreated)
    {
        _eventQueue.Enqueue([entityTemplate, onCreated, preInitialize, this](World* world)
        {
            Entity& entity = world->CreateEntityInternal(entityTemplate);
            preInitialize(entity);

            OnEntityCreated(entity, entityTemplate);
            
            onCreated(entity);
        });
    }
    
    template <typename Func>
    void CreateEntityAsync(const std::shared_ptr<EntityTemplate>& entityTemplate, uint32 count, Func onCreated)
    {
        _eventQueue.Enqueue([entityTemplate, onCreated, count](World* world)
        {
            for (uint32 i = 0; i < count; ++i)
            {
                Entity& newEntity = world->CreateEntity(entityTemplate);
                onCreated(newEntity);
            }
        });
    }
    
    Entity& CreateEntity(const Archetype& archetype);
    void CreateEntities(const Archetype& archetype, uint32 count);

    void DestroyEntity(Entity& entity);

    AddComponentResult<Component> AddComponent(Entity& entity, Type& componentType, Name name);

    template <typename ComponentType> requires IsA<ComponentType, Component>
    AddComponentResult<ComponentType> AddComponent(Entity& entity, Name name)
    {
        auto [component, newEntity] = AddComponent(entity, *ComponentType::StaticType(), name);
        return {std::dynamic_pointer_cast<ComponentType>(component), newEntity};
    }

    void RemoveComponent(Entity& entity, uint16 index);

    template <typename ComponentType> requires IsA<ComponentType, Component>
    ComponentType& Get(Entity& entity, const Archetype& archetype)
    {
        uint16 index = archetype.GetComponentIndex<ComponentType>();
        ComponentType& component = entity.Get<ComponentType>(index);
        
        if constexpr (RequiresOnChanged<ComponentType>)
        {
            EventDispatcher<TypeSet<ComponentType>>& event = this->*ComponentType::OnChanged;
            event.Add(entity, archetype, {});
        }
        
        return component;
    }

    template <typename SystemType> requires IsA<SystemType, SystemBase>
    SystemType& AddSystem()
    {
        SystemType& newSystem = _systemScheduler.AddSystem<SystemType>();
        newSystem.SetWorld(this, {});
        newSystem.UpdateQuery({});
        newSystem.CallInitialize({});
        return newSystem;
    }

    template <typename SystemType> requires IsA<SystemType, SystemBase>
    void AddSystemAsync()
    {
        _eventQueue.Enqueue([this](World* world)
        {
            AddSystem<SystemType>();
        });
    }

    template <typename SystemType, typename Func> requires IsA<SystemType, SystemBase>
    void AddSystemAsync(Func onAdded)
    {
        _eventQueue.Enqueue([this, onAdded](World* world)
        {
            SystemType& newSystem = AddSystem<SystemType>();
            onAdded(newSystem);
        });
    }

    template <typename SystemType> requires IsA<SystemType, SystemBase>
    SystemType* FindSystem() const
    {
        for (const std::unique_ptr<SystemBase>& systemBase : _systemScheduler.GetSystems())
        {
            if (SystemType* system = dynamic_cast<SystemType*>(systemBase.get()))
            {
                return system;
            }
        }
        
        return nullptr;
    }

    void Query(ECSQuery& query, const Archetype& archetype) const;

    template <typename... ComponentType> requires (IsA<ComponentType, Component> && ...)
    void Query(ECSQuery& query)
    {
        const Archetype archetype = Archetype::Create<ComponentType...>();
        Query(query, archetype);
    }

    void Initialize(PassKey<GameplaySubsystem>);
    void Tick(double deltaTime, PassKey<GameplaySubsystem>);
    void Shutdown(PassKey<GameplaySubsystem>);

    EventQueue<World>& GetEventQueue();

    EventManager& GetEventManager();

    DirtyTracker& GetDirtyTracker(Type& componentType);

    template <typename ComponentType> requires IsA<ComponentType, Component>
    DirtyTracker& GetDirtyTracker()
    {
        return GetDirtyTracker(*ComponentType::StaticType());
    }

private:
    ObjectTypeMap _componentTypeMap;
    EntityListGraph _entityListGraph;
    SystemScheduler _systemScheduler;

    EventQueue<World> _eventQueue;

    EventManager _eventManager;

    std::unordered_map<Type*, DirtyTracker> _dirtyTrackers;

private:
    Entity& CreateEntityInternal(const Archetype& archetype);
    Entity& CreateEntityInternal(const std::shared_ptr<EntityTemplate>& entityTemplate);
    
    void OnEntityCreated(Entity& entity, const Archetype& archetype) const;
    void OnEntityCreated(Entity& entity, const std::shared_ptr<EntityTemplate>& archetype) const;
    SharedObjectPtr<Component> AddComponentInternal(Entity& entity, Type& componentType, Name name);

    EntityList& GetEntityList(const Archetype& archetype);
};
