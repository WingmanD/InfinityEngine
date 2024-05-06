#include "PointLightSystem.h"
#include "ECS/Entity.h"
#include "ECS/EntityList.h"

class CStaticMesh;

DynamicGPUBuffer2<PointLight>& PointLightSystem::GetPointLightBuffer()
{
    return _pointLightBuffer;
}

void PointLightSystem::Initialize()
{
    System::Initialize();

    _pointLightBuffer.Initialize();
}

void PointLightSystem::OnEntityCreated(const Archetype& archetype, Entity& entity)
{
    System::OnEntityCreated(archetype, entity);

    CTransform& transform = entity.Get<CTransform>(archetype);
    CPointLight& pointLight = entity.Get<CPointLight>(archetype);

    pointLight.LightTransform.SetParent(&transform.ComponentTransform);

    PointLight light;
    light.Location = transform.ComponentTransform.GetWorldLocation();
    light.Color = pointLight.Color;
    light.Intensity = pointLight.Intensity;
    light.Radius = pointLight.Radius;
    
    pointLight.LightID = static_cast<uint32>(_pointLightBuffer.Add(light));
    _registeredPointLightComponents.Add(&pointLight);
}

void PointLightSystem::Tick(double deltaTime)
{
    System::Tick(deltaTime);

    for (EntityList* entityList : GetQuery().GetEntityLists())
    {
        const Archetype& archetype = entityList->GetArchetype();
        entityList->ForEach([&archetype, this](Entity& entity)
        {
            const CPointLight& pointLight = entity.Get<CPointLight>(archetype);

            // todo optimize, this should be an event
            _pointLightBuffer[pointLight.LightID].Location = pointLight.LightTransform.GetWorldLocation();

            return true;
        });
    }
}

void PointLightSystem::OnEntityDestroyed(const Archetype& archetype, Entity& entity)
{
    System::OnEntityDestroyed(archetype, entity);

    const CPointLight& pointLight = entity.Get<CPointLight>(archetype);
    
    _pointLightBuffer.RemoveAtSwap(pointLight.LightID);

    _registeredPointLightComponents.Back()->LightID = pointLight.LightID;

    _registeredPointLightComponents.RemoveAtSwap(pointLight.LightID);
}

void PointLightSystem::Shutdown()
{
    System::Shutdown();
}
