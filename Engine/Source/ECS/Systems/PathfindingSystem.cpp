#include "PathfindingSystem.h"
#include "ECS/EntityList.h"
#include "Math/Math.h"

void PathfindingSystem::OnEntityCreated(const Archetype& archetype, Entity& entity)
{
    CPathfinding& pathfinding = entity.Get<CPathfinding>(archetype);
    const CTransform& transform = entity.Get<CTransform>(archetype);
    
    pathfinding.Destination = transform.ComponentTransform.GetWorldLocation() + Math::RandomUnitVector() * Math::Random(0.0f, 10.0f);
}

void PathfindingSystem::Tick(double deltaTime)
{
    for (EntityList* entityList : GetQuery().GetEntityLists())
    {
        const Archetype& archetype = entityList->GetArchetype();
        entityList->ForEach([&archetype, deltaTime](Entity& entity)
        {
            const CPathfinding& pathfinding = entity.Get<CPathfinding>(archetype);
            CTransform& transform = entity.Get<CTransform>(archetype);

            if (Vector3::Distance(transform.ComponentTransform.GetWorldLocation(), pathfinding.Destination) < 0.1f)
            {
                return true;
            }

            Vector3 direction = pathfinding.Destination - transform.ComponentTransform.GetWorldLocation();
            direction.Normalize();

            transform.ComponentTransform.SetWorldLocation(
                transform.ComponentTransform.GetWorldLocation() + direction * pathfinding.Speed * static_cast<float>(deltaTime));

            return true;
        });
    }
}
