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
    void CallTick(double deltaTime, PassKey<SystemScheduler>);
    void CallShutdown(PassKey<SystemScheduler>);

    void SetWorld(World* world, PassKey<World>);
    World& GetWorld() const;
    
    void UpdateQuery(PassKey<World>);

    const Archetype& GetArchetype() const;

protected:
    virtual void Initialize() = 0;
    virtual void Tick(double deltaTime) = 0;
    virtual void Shutdown() = 0;

    const ECSQuery& GetQuery() const;
    

private:
    Archetype _archetype;
    ECSQuery _persistentQuery;
    World* _world = nullptr;
};

template <typename... ComponentTypes> requires (IsReflectedType<ComponentTypes> && ...)
class System
{
public:
    System() : System(Archetype::Create<ComponentTypes...>())
    {
    }
};
