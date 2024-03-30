#pragma once

#include "ECS/Archetype.h"
#include "ECS/ECSQuery.h"

class SystemScheduler;
class World;

class SystemBase
{
public:
    SystemBase(Archetype&& archetype);

    SystemBase(const SystemBase&) = delete;
    SystemBase(SystemBase&&) = delete;

    SystemBase& operator=(const SystemBase&) = delete;
    SystemBase& operator=(SystemBase&&) = delete;

    virtual ~SystemBase() = default;
    
    void CallInitialize(PassKey<SystemScheduler>);
    void CallOnEntityCreated(const Archetype& archetype, Entity& entity, PassKey<World>);
    void CallTick(double deltaTime, PassKey<SystemScheduler>);
    void CallOnEntityDestroyed(const Archetype& archetype, Entity& entity, PassKey<World>);
    void CallShutdown(PassKey<SystemScheduler>);

    void SetWorld(World* world, PassKey<World>);
    World& GetWorld() const;
    
    void UpdateQuery(PassKey<World>);

    const Archetype& GetArchetype() const;

protected:
    virtual void Initialize();
    virtual void OnEntityCreated(const Archetype& archetype, Entity& entity);   // todo refactor, this call should be scheduled
    virtual void Tick(double deltaTime) = 0;
    virtual void OnEntityDestroyed(const Archetype& archetype, Entity& entity);
    virtual void Shutdown();

    const ECSQuery& GetQuery() const;

private:
    Archetype _archetype;
    ECSQuery _persistentQuery;
    World* _world = nullptr;
};

template <typename... ComponentTypes> requires (IsReflectedType<ComponentTypes> && ...)
class System : public SystemBase
{
public:
    System() : SystemBase(Archetype::Create<ComponentTypes...>())
    {
    }
};
