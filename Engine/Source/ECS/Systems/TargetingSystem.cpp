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

    entityList.ForEach([this](Entity& entity)
    {
        CTargeting& targeting = Get<CTargeting>(entity);
        if (targeting.Target != nullptr && targeting.Target->IsValid())
        {
            CPathfinding& pathfinding = Get<CPathfinding>(entity);
            const Transform& targetTransform = targeting.Target->Get<CTransform>(targeting.TargetArchetype).ComponentTransform;

            const Vector3 targetLocation = targetTransform.GetWorldLocation();
            pathfinding.Destination = targetLocation;

            const CTransform& transform = Get<const CTransform>(entity);
            const Vector3 location = transform.ComponentTransform.GetWorldLocation();
            if (Vector3::Distance(targetLocation, location) <= targeting.Range)
            {
                GetWorld().FindSystem<HealthSystem>()->DamageEntity(*targeting.Target, targeting.TargetArchetype, 10.0f);
                // GetWorld().CreateEntityAsync(
                //     targeting.ProjectileTemplate,
                //     [location](Entity& entity, const Archetype& archetype)
                //     {
                //         CTransform& transform = entity.Get<CTransform>(archetype);
                //         transform.ComponentTransform.SetWorldLocation(location);
                //     },
                //     [](Entity& entity, const Archetype& archetype)
                //     {
                //     }
                // );
            }
        }
        else
        {
            ECSQuery query;
            GetWorld().Query(query, Archetype::Create<CTeamMember>());

            const uint32 myTeam = Get<const CTeamMember>(entity).TeamID;
            
            for (EntityList* entityList : query.GetEntityLists())
            {
                entityList->ForEach([&entityList, this, myTeam, &entity](Entity& otherEntity)
                {
                    const CTeamMember teamMember = otherEntity.Get<const CTeamMember>(entityList->GetArchetype());
                    if (teamMember.TeamID != myTeam)
                    {
                        if (Math::Random(0.0f, 1.0f) > 0.75f)
                        {
                            CTargeting& targeting = Get<CTargeting>(entity);
                            targeting.Target = &otherEntity;
                            targeting.TargetArchetype = entityList->GetArchetype();
                            return false;
                        }
                    }
                    
                    return true;
                });
            } 
            
            // const CTransform& transform = Get<const CTransform>(entity);
            
            // GetWorld().FindSystem<PhysicsSystem>()->ForEachEntityInSphere(
            //     transform.ComponentTransform.GetWorldLocation(),
            //     targeting.Range,
            //     [this, &entity](const Entity& target)
            //     {
            //         Archetype targetArchetype = Archetype(entity);
            //         const CTeamMember* teamMember = entity.GetChecked<CTeamMember>(targetArchetype);
            //         if (teamMember != nullptr)
            //         {
            //             if (teamMember->TeamID != Get<const CTeamMember>(entity).TeamID)
            //             {
            //                 CTargeting& targeting = Get<CTargeting>(entity);
            //                 targeting.Target = const_cast<Entity*>(&target);
            //                 targeting.TargetArchetype = targetArchetype;
            //                 return false;
            //             }
            //         }
            //
            //         return true;
            //     },
            //     PassKey<TargetingSystem>()
            // );
        }


        return true;
    });
}
