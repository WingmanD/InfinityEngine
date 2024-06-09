#include "LevelStreamingSystem.h"
#include "Level.h"
#include "ECS/EntityTemplate.h"

LevelStreamingSystem::LevelStreamingSystem(const LevelStreamingSystem& other) : System(other)
{
}

void LevelStreamingSystem::SetLevel(const std::shared_ptr<Level>& level)
{
    _level = level;
    _level->Load();
}

std::shared_ptr<Level> LevelStreamingSystem::GetLevel() const
{
    return _level;
}

void LevelStreamingSystem::ProcessEntityList(EntityList& entityList, double deltaTime)
{
    System::ProcessEntityList(entityList, deltaTime);

    if (_level == nullptr)
    {
        return;
    }

    entityList.ForEach([this](Entity& entity)
    {
        const CTransform& transform = Get<const CTransform>(entity);
        const CLevelStreamingInvoker& invoker = Get<const CLevelStreamingInvoker>(entity);

        _level->Stream(
            transform.ComponentTransform.GetWorldLocation(),
            invoker.StreamingDistance,
            [this](const Level::Chunk& chunk)
            {
                GetEventQueue().Enqueue([this, &chunk](SystemBase* system)
                {
                    OnChunkLoaded(chunk);
                });
            },
            _first
        );

        _first = false;

        return true;
    });
}

void LevelStreamingSystem::OnChunkLoaded(const Level::Chunk& chunk) const
{
    World& world = GetWorld();
    AssetManager& assetManager = AssetManager::Get();
    
    for (const Level::EntityElement& entityElement : chunk.EntityElements)
    {
        std::shared_ptr<EntityTemplate> entityTemplate = assetManager.FindAsset<EntityTemplate>(entityElement.EntityTemplateID);
        if (entityTemplate == nullptr)
        {
            continue;
        }

        const Transform& entityTransform = entityElement.EntityTransform;

        world.CreateEntityAsync(
            entityTemplate,
            [entityTemplate, entityTransform](Entity& entity)
            {
                entity.Get<CTransform>(entityTemplate->GetArchetype()).ComponentTransform = entityTransform;
            },
            [](Entity& entity)
            {
            }
        );
    }
}
