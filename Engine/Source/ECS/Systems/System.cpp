#include "System.h"

#include "ECS/Event.h"
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
    std::ignore = GetType()->ForEachProperty([this](PropertyBase* propertyBase)
    {
        Property<SystemBase, EventBase>* property = dynamic_cast<Property<SystemBase, EventBase>*>(propertyBase);
        if (property == nullptr)
        {
            return true;
        }

        EventBase& valueRef = property->GetRef(this);
        valueRef.SetEventManager(GetWorld().GetEventManager());
        
        return true;
    });
}

void SystemBase::OnEntityCreated(const Archetype& archetype, Entity& entity)
{
}

void SystemBase::Tick(double deltaTime)
{
}

void SystemBase::ProcessEntityList(EntityList& entityList, double deltaTime)
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

DirtyTracker& SystemBase::GetDirtyTracker(Type& componentType) const
{
    return GetWorld().GetDirtyTracker(componentType);
}
