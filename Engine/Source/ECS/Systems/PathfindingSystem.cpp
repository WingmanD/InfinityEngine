#include "PathfindingSystem.h"
#include "ECS/EntityList.h"
#include "ECS/Components/CStaticMesh.h"
#include "Math/Math.h"

void PathfindingSystem::OnEntityCreated(const Archetype& archetype, Entity& entity)
{
    CPathfinding& pathfinding = entity.Get<CPathfinding>(archetype);
    const CTransform& transform = entity.Get<CTransform>(archetype);
    
    pathfinding.Destination = transform.ComponentTransform.GetWorldLocation() + Math::RandomUnitVector() * Math::Random(0.0f, 10.0f);
}

void PathfindingSystem::ProcessEntityList(EntityList& entityList, double deltaTime)
{
    System::ProcessEntityList(entityList, deltaTime);

    entityList.ForEach([this, deltaTime](Entity& entity)
    {
        const CPathfinding& pathfinding = Get<const CPathfinding>(entity);

        {
            const CTransform& transform = Get<const CTransform>(entity);

            if (Vector3::Distance(transform.ComponentTransform.GetWorldLocation(), pathfinding.Destination) < 0.1f)
            {
                return true;
            }
        }

        CTransform& transform = Get<CTransform>(entity);
        
        Vector3 direction = pathfinding.Destination - transform.ComponentTransform.GetWorldLocation();
        direction.Normalize();

        const Vector3 currentLocation = transform.ComponentTransform.GetWorldLocation();
        const Vector3 newLocation = currentLocation + direction * pathfinding.Speed * static_cast<float>(deltaTime);
        transform.ComponentTransform.SetWorldLocation(newLocation);

        return true;
    });
}
