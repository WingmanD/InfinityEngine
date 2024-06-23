#include "SpawnerSystem.h"
#include "ECS/EntityTemplate.h"
#include "ECS/Components/CTeamMember.h"
#include "ECS/Components/CTransform.h"

SpawnerSystem::SpawnerSystem(const SpawnerSystem& other) : System(other)
{
}

void SpawnerSystem::OnEntityCreated(const Archetype& archetype, Entity& entity)
{
    System::OnEntityCreated(archetype, entity);

    CTransform& transform = entity.Get<CTransform>(archetype);
    CSpawner& spawner = entity.Get<CSpawner>(archetype);

    spawner.SpawnTransform.SetParent(&transform.ComponentTransform);
}

void SpawnerSystem::ProcessEntityList(EntityList& entityList, double deltaTime)
{
    System::ProcessEntityList(entityList, deltaTime);

    entityList.ForEach([this, deltaTime](Entity& entity)
    {
        CSpawner& spawner = Get<CSpawner>(entity);

        if (spawner.SpawnCount >= spawner.TargetSpawnCount)
        {
            return true;
        }

        spawner.SpawnTimer += static_cast<float>(deltaTime);
        
        if (spawner.SpawnTimer >= 1.0f / spawner.SpawnRate)
        {
            spawner.SpawnTimer = 0.0;

            AssetPtr<EntityTemplate> spawnTemplate = spawner.SpawnTemplate;
            const Transform& spawnTransform = spawner.SpawnTransform;
            const uint32 teamID = Get<const CTeamMember>(entity).TeamID;
            
            GetWorld().CreateEntityAsync(
                spawner.SpawnTemplate,
                [spawnTransform, teamID, spawnTemplate](Entity& newEntity, const Archetype& archetype)
                {
                    CTransform& transform = newEntity.Get<CTransform>(archetype);
                    transform.ComponentTransform.SetWorldLocation(spawnTransform.GetWorldLocation());

                    CTeamMember* teamMember = newEntity.GetChecked<CTeamMember>(archetype);
                    if (teamMember != nullptr)
                    {
                        teamMember->TeamID = teamID;
                    }
                },
                [](Entity& entity, const Archetype& archetype)
                {
                }
            );

            ++spawner.SpawnCount;
        }

        return true;
    });
}
