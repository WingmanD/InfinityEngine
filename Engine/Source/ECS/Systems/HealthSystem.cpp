#include "ECS/Systems/HealthSystem.h"
#include "ECS/World.h"
#include "ECS/Components/CHealth.h"

HealthSystem::HealthSystem(const HealthSystem& other) : System(other)
{
}

void HealthSystem::DamageEntity(Entity& entity, const Archetype& archetype, float damage)
{
    _onEntityDamaged.Add(entity, archetype, damage, PassKey<HealthSystem>());
}

void HealthSystem::Tick(double deltaTime)
{
    GetEventQueue().ProcessEvents();

    for (auto& entityListStruct : _onEntityDamaged.GetEntityLists())
    {
        if (!entityListStruct.EntityArchetype.HasComponent<CHealth>())
        {
            continue;
        }
        
        CacheArchetype(entityListStruct.EntityArchetype);

        World& world = GetWorld();

        Event<TypeSet<>, float>::EventData eventData;
        while (entityListStruct.Queue.Dequeue(eventData))
        {
            if (!eventData.Entity->IsValid() || eventData.Entity->GetID() != eventData.ID)
            {
                continue;
            }

            CHealth& health = Get<CHealth>(*eventData.Entity);
            health.Health -= std::get<float>(eventData.Arguments);

            if (health.Health <= 0.0f)
            {
                world.DestroyEntityAsync(*eventData.Entity);
            }
        }
    }
}
