#include "System.h"

SystemBase::SystemBase(Archetype&& archetype) : _archetype(std::move(archetype))
{
}

void SystemBase::CallInitialize(PassKey<SystemScheduler>)
{
    Initialize();
}

void SystemBase::CallTick(double deltaTime, PassKey<SystemScheduler>)
{
    Tick(deltaTime);
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

const ECSQuery& SystemBase::GetQuery() const
{
    return _persistentQuery;
}
