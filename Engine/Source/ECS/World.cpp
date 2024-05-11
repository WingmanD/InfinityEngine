#include "World.h"
#include "EntityTemplate.h"

World::World() : _eventQueue(this)
{
}

World::World(const World&) : Object(), _eventQueue(this)
{
}

void World::CreateEntityAsync(const Archetype& archetype)
{
    _eventQueue.Enqueue([this, archetype](World* world)
    {
        CreateEntity(archetype);
    });
}

void World::CreateEntityAsync(const Archetype& archetype, uint32 count)
{
    _eventQueue.Enqueue([this, archetype, count](World* world)
    {
        for (uint32 i = 0; i < count; ++i)
        {
            CreateEntity(archetype);
        }
    });
}

void World::DestroyEntityAsync(Entity& entity)
{
    _eventQueue.Enqueue([this, &entity](World* world)
    {
        DestroyEntity(entity);
    });
}

Entity& World::CreateEntity(const std::shared_ptr<EntityTemplate>& entityTemplate)
{
    assert(entityTemplate != nullptr);

    const Archetype& archetype = entityTemplate->GetArchetype();
    
    Entity& entity = CreateEntityInternal(archetype);
    entityTemplate->InitializeEntity(entity);
    
    OnEntityCreated(entity, archetype);

    return entity;
}

void World::CreateEntity(const std::shared_ptr<EntityTemplate>& entityTemplate, uint32 count)
{
    for (uint32 i = 0; i < count; ++i)
    {
        CreateEntity(entityTemplate);
    }
}

Entity& World::CreateEntity(const Archetype& archetype)
{
    Entity& entity = CreateEntityInternal(archetype);
    OnEntityCreated(entity, archetype);

    return entity;
}

void World::CreateEntities(const Archetype& archetype, uint32 count)
{
    for (uint32 i = 0; i < count; ++i)
    {
        CreateEntity(archetype);
    }
}

void World::DestroyEntity(Entity& entity)
{
    const Archetype archetype = Archetype(entity);
    const EntityListGraph::EntityListResult result = _entityListGraph.GetOrCreateEntityListFor(archetype);
    EntityList& entityList = *result.List;

    for (const std::unique_ptr<SystemBase>& system : _systemScheduler.GetSystems())
    {
        if (system->GetArchetype().IsSubsetOf(archetype))
        {
            system->CallOnEntityDestroyed(archetype, entity, {});
        }
    }

    entityList.Remove(entity);
}

SharedObjectPtr<Component> World::AddComponent(Entity& entity, Type& componentType, Name name)
{
    Archetype archetypeBefore = Archetype(entity);
    const SharedObjectPtr<Component> newComponent = AddComponentInternal(entity, componentType, name);
    Archetype archetypeAfter = Archetype(entity);

    EntityList& entityListBefore = GetEntityList(archetypeBefore);
    EntityList& entityListAfter = GetEntityList(archetypeAfter);

    entityListAfter.Add(entity);
    entityListBefore.Remove(entity);

    return newComponent;
}

void World::RemoveComponent(Entity& entity, uint16 index)
{
    Archetype archetypeBefore = Archetype(entity);
    entity.RemoveComponent(index, {});
    Archetype archetypeAfter = Archetype(entity);

    EntityList& entityListBefore = GetEntityList(archetypeBefore);
    EntityList& entityListAfter = GetEntityList(archetypeAfter);

    entityListAfter.Add(entity);
    entityListBefore.Remove(entity);
}

void World::Query(ECSQuery& query, const Archetype& archetype) const
{
    _entityListGraph.Query(query, archetype);
}

void World::Initialize(PassKey<GameplaySubsystem>)
{
    std::ignore = GetType()->ForEachProperty([this](PropertyBase* propertyBase)
    {
        Property<World, EventBase>* property = static_cast<Property<World, EventBase>*>(propertyBase);
        if (property == nullptr)
        {
            return true;
        }

        EventBase& valueRef = property->GetRef(this);
        valueRef.SetEventManager(GetEventManager());
            
        return true;
    });
}

void World::Tick(double deltaTime, PassKey<GameplaySubsystem>)
{
    _eventQueue.ProcessEvents();

    _systemScheduler.Tick(deltaTime);

    _eventQueue.ProcessEvents();
    
    OnTransformChanged.Clear();
}

void World::Shutdown(PassKey<GameplaySubsystem>)
{
    _systemScheduler.Shutdown();
}

EventQueue<World>& World::GetEventQueue()
{
    return _eventQueue;
}

EventManager& World::GetEventManager()
{
    return _eventManager;
}

DirtyTracker& World::GetDirtyTracker(Type& componentType)
{
    return _dirtyTrackers[&componentType];
}

Entity& World::CreateEntityInternal(const Archetype& archetype)
{
    EntityList& entityList = GetEntityList(archetype);

    Entity& entity = *entityList.AddDefault();
    for (const Archetype::QualifiedComponentType& qualifiedType : archetype.GetComponentTypes())
    {
        AddComponentInternal(entity, *qualifiedType.Type, qualifiedType.Name);
    }

    return entity;
}

void World::OnEntityCreated(Entity& entity, const Archetype& archetype) const
{
    for (const std::unique_ptr<SystemBase>& system : _systemScheduler.GetSystems())
    {
        if (archetype.IsSubsetOf(system->GetArchetype()))
        {
            system->CallOnEntityCreated(archetype, entity, {});
        }
    }
}

SharedObjectPtr<Component> World::AddComponentInternal(Entity& entity, Type& componentType, Name name)
{
    const SharedObjectPtr<Component> newComponent = _componentTypeMap.NewObject<Component>(componentType);
    newComponent->SetName(name, {});
    entity.AddComponent(newComponent, {});

    return newComponent;
}

EntityList& World::GetEntityList(const Archetype& archetype)
{
    const EntityListGraph::EntityListResult result = _entityListGraph.GetOrCreateEntityListFor(archetype);

    if (result.WasCreated)
    {
        GetEventManager().UpdateQueries(_entityListGraph);
        
        for (const std::unique_ptr<SystemBase>& system : _systemScheduler.GetSystems())
        {
            if (system->GetArchetype().IsSubsetOf(archetype))
            {
                system->UpdateQuery({});
            }
        }
    }

    return *result.List;
}
