#include "CameraSystem.h"
#include "ECS/Entity.h"

void CameraSystem::OnEntityCreated(const Archetype& archetype, Entity& entity)
{
    CTransform* transform = entity.Get<CTransform>(archetype);
    CCamera* camera = entity.Get<CCamera>(archetype);

    camera->CameraTransform.SetParent(&transform->ComponentTransform);
}
