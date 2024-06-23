#include "ECS/Systems/ProjectileSystem.h"

#include "HealthSystem.h"
#include "ECS/Systems/PhysicsSystem.h"

ProjectileSystem::ProjectileSystem(const ProjectileSystem& other) : System(other)
{
}

void ProjectileSystem::Initialize()
{
    System::Initialize();

    _onHitHandle = GetWorld().FindSystem<PhysicsSystem>()->OnHit.RegisterListener(_onHit);
}

void ProjectileSystem::OnEntityCreated(const Archetype& archetype, Entity& entity)
{
    System::OnEntityCreated(archetype, entity);

    CacheArchetype(archetype);

    const CProjectile& projectile = Get<const CProjectile>(entity);
    CRigidBody& rigidBody = Get<CRigidBody>(entity);
    const CTransform& transform = Get<const CTransform>(entity);

    rigidBody.Velocity = transform.ComponentTransform.GetForwardVector() * projectile.Speed;
}

void ProjectileSystem::Tick(double deltaTime)
{
    System::Tick(deltaTime);

    HealthSystem* healthSystem = GetWorld().FindSystem<HealthSystem>();

    for (auto& entityListStruct : _onHit.GetEntityLists())
    {
        PhysicsSystem::EventHit::EventData eventData;
        while (entityListStruct.Queue.Dequeue(eventData))
        {
            if (!eventData.Entity->IsValid() || eventData.Entity->GetID() != eventData.ID)
            {
                continue;
            }

            const PhysicsSystem::Hit& hit = std::get<PhysicsSystem::Hit>(eventData.Arguments);
            
            if (const CProjectile* projectile = eventData.Entity->GetChecked<CProjectile>(entityListStruct.EntityArchetype))
            {
                if (projectile->Damage > 0.0f)
                {
                    if (Archetype(*hit.OtherBody->Entity).HasComponent<CHealth>())
                    {
                        healthSystem->DamageEntity(*hit.OtherBody->Entity, entityListStruct.EntityArchetype, projectile->Damage);
                    }
                }

                GetWorld().DestroyEntityAsync(*eventData.Entity);
            }
        }
    }
}

void ProjectileSystem::Shutdown()
{
    System::Shutdown();

    GetWorld().FindSystem<PhysicsSystem>()->OnHit.UnregisterListener(_onHitHandle);
}
