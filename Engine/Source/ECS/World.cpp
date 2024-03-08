#include "World.h"

Entity& World::CreateEntity(const Archetype& archetype)
{
    EntityList& entityList = _entityListGraph.GetOrCreateEntityListFor(archetype);

    Entity& entity = *entityList.AddDefault();
    for (Type* type : archetype.GetComponentTypes())
    {
        AddComponent(entity, *type);
    }

    return entity;
}

Entity& World::RegisterEntity(const Archetype& archetype, Entity&& entity)
{
    EntityList& entityList = _entityListGraph.GetOrCreateEntityListFor(archetype);
    return *entityList.Emplace(std::move(entity));
}

void World::DestroyEntity(const Archetype& archetype, Entity& entity)
{
    EntityList& entityList = _entityListGraph.GetOrCreateEntityListFor(archetype);
    entityList.Remove(entity);
}

SharedObjectPtr<Component> World::AddComponent(Entity& entity, Type& componentType)
{
    const SharedObjectPtr<Component> newComponent = _componentTypeMap.NewObject<Component>(componentType);
    entity.AddComponent(newComponent, {});

    return newComponent;
}

void World::RemoveComponent(Entity& entity, uint16 index)
{
    entity.RemoveComponent(index, {});
}

void World::Tick(double deltaTime, PassKey<GameplaySubsystem>)
{
}

void World::SetValidImplementation(bool valid)
{
    _isValid = valid;
}

bool World::IsValidImplementation() const
{
    return _isValid;
}
