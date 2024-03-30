#include "World.h"
#include "EntityTemplate.h"

World::World() : _eventQueue(this)
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
    Entity& entity = CreateEntity(entityTemplate->GetArchetype());
    entityTemplate->InitializeEntity(entity);
    
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
    EntityList& entityList = GetEntityList(archetype);

    Entity& entity = *entityList.AddDefault();
    for (const Archetype::QualifiedComponentType& qualifiedType : archetype.GetComponentTypes())
    {
        AddComponentInternal(entity, *qualifiedType.Type, qualifiedType.Name);
    }

    for (const std::unique_ptr<SystemBase>& system : _systemScheduler.GetSystems())
    {
        if (system->GetArchetype().IsSubsetOf(archetype) || system->GetArchetype().IsSupersetOf(archetype))
        {
            system->CallOnEntityCreated(archetype, entity, {});
        }
    }

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
        if (system->GetArchetype().IsSubsetOf(archetype) || system->GetArchetype().IsSupersetOf(archetype))
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

void World::Query(ECSQuery& query, const Archetype& archetype)
{
    _entityListGraph.Query(query, archetype);
}

void World::Initialize()
{
}

void World::Tick(double deltaTime, PassKey<GameplaySubsystem>)
{
    _eventQueue.ProcessEvents();

    _systemScheduler.Tick(deltaTime);

    _eventQueue.ProcessEvents();
}

void World::Shutdown()
{
    _systemScheduler.Shutdown();
}

EventQueue<World>& World::GetEventQueue()
{
    return _eventQueue;
}

void World::SetValidImplementation(bool valid)
{
    _isValid = valid;
}

bool World::IsValidImplementation() const
{
    return _isValid;
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
        for (const std::unique_ptr<SystemBase>& system : _systemScheduler.GetSystems())
        {
            if (system->GetArchetype().IsSubsetOf(archetype) || system->GetArchetype().IsSupersetOf(archetype))
            {
                system->UpdateQuery({});
            }
        }
    }

    return *result.List;
}
