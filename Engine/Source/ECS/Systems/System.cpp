#include "System.h"
#include "ECS/World.h"

SystemBase::SystemBase(Archetype&& archetype) : _archetype(std::move(archetype))
{
}

void SystemBase::CallInitialize(PassKey<SystemScheduler>)
{
    Initialize();
}

void SystemBase::CallOnEntityCreated(const Archetype& archetype, Entity& entity, PassKey<World>)
{
    OnEntityCreated(archetype, entity);
}

void SystemBase::CallTick(double deltaTime, PassKey<SystemScheduler>)
{
    Tick(deltaTime);
}

void SystemBase::CallOnEntityDestroyed(const Archetype& archetype, Entity& entity, PassKey<World>)
{
    OnEntityDestroyed(archetype, entity);
}

void SystemBase::CallShutdown(PassKey<SystemScheduler>)
{
    Shutdown();
}

void SystemBase::SetWorld(World* world, PassKey<World>)
{
    _world = world;
}

World& SystemBase::GetWorld() const
{
    return *_world;
}

void SystemBase::UpdateQuery(PassKey<World>)
{
    _world->Query(_persistentQuery, _archetype);
}

const Archetype& SystemBase::GetArchetype() const
{
    return _archetype;
}

void SystemBase::Initialize()
{
}

void SystemBase::OnEntityCreated(const Archetype& archetype, Entity& entity)
{
}

void SystemBase::Tick(double deltaTime)
{
}

void SystemBase::OnEntityDestroyed(const Archetype& archetype, Entity& entity)
{
}

void SystemBase::Shutdown()
{
}

const ECSQuery& SystemBase::GetQuery() const
{
    return _persistentQuery;
}
