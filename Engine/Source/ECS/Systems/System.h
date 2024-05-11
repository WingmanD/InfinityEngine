#pragma once

#include "TypeMap.h"
#include "TypeSet.h"
#include "ECS/Archetype.h"
#include "ECS/DirtyTracker.h"
#include "ECS/ECSQuery.h"
#include "ECS/EntityList.h"
#include "Object.h"
#include "ECS/Event.h"
#include "ECS/Systems/System.reflection.h"

class SystemScheduler;
class World;

template <typename T>
concept RequiresOnChanged = requires
{
    { T::OnChanged };
};

REFLECTED()
class SystemBase : public Object
{
    GENERATED()
    
public:
    explicit SystemBase() = default;
    explicit SystemBase(Archetype&& archetype);

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
    virtual void OnEntityCreated(const Archetype& archetype, Entity& entity);
    // todo refactor, this call should be scheduled
    virtual void Tick(double deltaTime);
    virtual void ProcessEntityList(EntityList& entityList, double deltaTime);
    virtual void OnEntityDestroyed(const Archetype& archetype, Entity& entity);
    virtual void Shutdown();

    const ECSQuery& GetQuery() const;

    DirtyTracker& GetDirtyTracker(Type& componentType) const;

private:
    Archetype _archetype;
    ECSQuery _persistentQuery;
    World* _world = nullptr;
};

template <typename... ComponentTypes> requires (IsA<ComponentTypes, Component> && ...)
class System : public SystemBase
{
public:
    using RequiredComponents = TypeSet<ComponentTypes...>;
    
public:
    System() : SystemBase(Archetype::Create<ComponentTypes...>())
    {
    }

protected:
    template <typename ComponentType> requires IsA<ComponentType, Component> && (IsA<ComponentType, ComponentTypes> || ...) && !std::is_const_v<ComponentType>
    ComponentType& Get(Entity& entity) const
    {
        ComponentType& component = entity.Get<ComponentType>(IndexOf<ComponentType>());
        
        if constexpr (RequiresOnChanged<ComponentType>)
        {
            if (!component.Dirty)
            {
                Event<TypeSet<ComponentType>>& event = GetWorld().*ComponentType::OnChanged;
                event.Add(&entity, *GetBinding<ComponentType>().ListArchetype, PassKey<System<ComponentTypes...>>());
            }
        }
        
        return component;
    }

    template <typename ComponentType> requires IsA<ComponentType, Component> && (IsA<ComponentType, ComponentTypes> || ...) && std::is_const_v<ComponentType>
    const ComponentType& Get(Entity& entity) const
    {
        return entity.Get<ComponentType>(IndexOf<ComponentType>());
    }

    // SystemBase
protected:
    virtual void Tick(double deltaTime) override
    {
        for (EntityList* entityList : GetQuery().GetEntityLists())
        {
            (UpdateBinding<ComponentTypes>(entityList->GetArchetype()), ...);

            ProcessEntityList(*entityList, deltaTime);
        }
    }

private:
    struct ComponentBinding
    {
        uint16 Index = 0;
        const Archetype* ListArchetype = nullptr;
    };

    TypeMap<ComponentBinding, ComponentTypes...> _componentBindings;

private:
    template <typename T> requires IsA<T, Component>
    constexpr uint16 IndexOf() const
    {
        return GetBinding<T>().Index;
    }

    template <typename T> requires IsA<T, Component>
    ComponentBinding& GetBinding()
    {
        return _componentBindings.template Get<T>();
    }

    template <typename T> requires IsA<T, Component>
    const ComponentBinding& GetBinding() const
    {
        return _componentBindings.template Get<T>();
    }

    template <typename T> requires IsA<T, Component>
    void UpdateBinding(const Archetype& archetype)
    {
        ComponentBinding& binding = GetBinding<T>();
        binding.Index = archetype.GetComponentIndex<T>();
        binding.ListArchetype = &archetype;
    }
};
