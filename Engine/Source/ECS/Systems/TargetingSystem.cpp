#include "ECS/Systems/TargetingSystem.h"
#include "ECS/Systems/HealthSystem.h"
#include "ECS/Systems/PhysicsSystem.h"
#include "Math/Math.h"

void TargetingSystem::OnEntityCreated(const Archetype& archetype, Entity& entity)
{
    System::OnEntityCreated(archetype, entity);

    CTargeting& targeting = Get<CTargeting>(entity);
    const CTransform& transform = Get<const CTransform>(entity);
    targeting.ProjectileSpawnOffset.SetParent(&transform.ComponentTransform);
}

void TargetingSystem::ProcessEntityList(EntityList& entityList, double deltaTime)
{
    System::ProcessEntityList(entityList, deltaTime);

    entityList.ForEach([this, deltaTime](Entity& entity)
    {
        CTargeting& targeting = Get<CTargeting>(entity);
        if (targeting.Target != nullptr && targeting.Target->IsValid() && targeting.Target->GetID() == targeting.TargetID)
        {
            CPathfinding& pathfinding = Get<CPathfinding>(entity);
            const Transform& targetTransform = targeting.Target->Get<const CTransform>(*targeting.TargetArchetype).ComponentTransform;

            const Vector3 targetLocation = targetTransform.GetWorldLocation();
            pathfinding.Destination = targetLocation;

            const CTransform& transform = Get<const CTransform>(entity);
            const Vector3 location = transform.ComponentTransform.GetWorldLocation();
            const Quaternion rotation = transform.ComponentTransform.GetWorldRotation();
            if (Vector3::Distance(targetLocation, location) <= targeting.Range)
            {
                if (targeting.TimeSinceLastShot > 1.0f / targeting.RateOfFire)
                {
                    GetWorld().FindSystem<HealthSystem>()->DamageEntity(*targeting.Target, *targeting.TargetArchetype, 10.0f);
                    GetWorld().CreateEntityAsync(
                        targeting.ProjectileTemplate,
                        [location, rotation](Entity& entity, const Archetype& archetype)
                        {
                            CTransform& transform = entity.Get<CTransform>(archetype);
                            transform.ComponentTransform.SetWorldLocation(location);
                            transform.ComponentTransform.SetWorldRotation(rotation);

                            return &entity;
                        },
                        [](Entity& entity, const Archetype& archetype)
                        {
                        }
                    );
                }
                else
                {
                    targeting.TimeSinceLastShot += static_cast<float>(deltaTime);
                }
            }
        }
        else
        {
            if (targeting.TargetingDelayTimer < targeting.TargetingDelay)
            {
                targeting.TargetingDelayTimer += static_cast<float>(deltaTime);
                return true;
            }

            const uint32 myTeam = Get<const CTeamMember>(entity).TeamID;

            CPathfinding& pathfinding = Get<CPathfinding>(entity);
            pathfinding.Destination = Get<const CTransform>(entity).ComponentTransform.GetWorldLocation();

            for (EntityList* entityList : GetQuery().GetEntityLists())
            {
                entityList->ForEach([&entityList, this, myTeam, &entity, &targeting](Entity& otherEntity)
                {
                    const CTeamMember teamMember = otherEntity.Get<const CTeamMember>(entityList->GetArchetype());
                    if (teamMember.TeamID != myTeam)
                    {
                        if (Math::Random(0.0f, 1.0f) > 0.75f || myTeam == 1)
                        {
                            targeting.Target = &otherEntity;
                            targeting.TargetID = otherEntity.GetID();
                            targeting.TargetArchetype = &entityList->GetArchetype();

                            return false;
                        }
                    }

                    return true;
                });
                
                if (targeting.Target != nullptr)
                {
                    break;
                }
            }
        }

        return true;
    });
}
