#include "PointLightSystem.h"
#include "ECS/Entity.h"

PointLightSystem::PointLightSystem(const PointLightSystem& other) : System(other)
{
}

DynamicGPUBuffer2<PointLight>& PointLightSystem::GetPointLightBuffer()
{
    return _pointLightBuffer;
}

void PointLightSystem::Initialize()
{
    System::Initialize();

    _pointLightBuffer.Initialize();

    _onTransformChangedHandle = GetWorld().OnTransformChanged.RegisterListener(_onTransformChanged);
    _onArchetypeChangedHandle = GetWorld().OnArchetypeChanged.RegisterListener(_onArchetypeChanged);
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
    for (auto& entityListStruct : _onArchetypeChanged.GetEntityLists())
    {
        const uint16 pointLightIndex = entityListStruct.EntityArchetype.GetComponentIndexChecked<CPointLight>();
        if (pointLightIndex == std::numeric_limits<uint16>::max())
        {
            continue;
        }

        EventArchetypeChanged::EventData eventData;
        while (entityListStruct.Queue.Dequeue(eventData))
        {
            CPointLight& oldPointLight = eventData.Entity->Get<CPointLight>(pointLightIndex);
            CPointLight& newPointLight = eventData.Entity->Get<CPointLight>(std::get<Archetype>(eventData.Arguments));
            
            _registeredPointLightComponents[oldPointLight.LightID] = &newPointLight;
        }
    }
    
    for (auto& entityListStruct : _onTransformChanged.GetEntityLists())
    {
        const uint16 index = entityListStruct.EntityArchetype.GetComponentIndexChecked<CPointLight>();
        if (index == std::numeric_limits<uint16>::max())
        {
            continue;
        }

        EventTransformChanged::EventData eventData;
        while (entityListStruct.Queue.Dequeue(eventData))
        {
            if (!eventData.Entity->IsValid() || eventData.Entity->GetID() != eventData.ID)
            {
                continue;
            }

            const CPointLight& pointLight = eventData.Entity->Get<CPointLight>(index);
            
            _pointLightBuffer[pointLight.LightID].Location = pointLight.LightTransform.GetWorldLocation();
        }
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

    World& world = GetWorld();
    world.OnTransformChanged.UnregisterListener(_onTransformChangedHandle);
    world.OnArchetypeChanged.UnregisterListener(_onArchetypeChangedHandle);
}
