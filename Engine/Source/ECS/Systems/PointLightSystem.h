#pragma once

#include "System.h"
#include "Containers/DynamicGPUBuffer2.h"
#include "ECS/Components/CPointLight.h"
#include "ECS/Components/CTransform.h"
#include "Rendering/PointLight.h"

class PointLightSystem : public System<const CTransform, const CPointLight>
{
public:
    DynamicGPUBuffer2<PointLight>& GetPointLightBuffer();

protected:
    virtual void Initialize() override;

    virtual void OnEntityCreated(const Archetype& archetype, Entity& entity) override;

    virtual void Tick(double deltaTime) override;

    virtual void OnEntityDestroyed(const Archetype& archetype, Entity& entity) override;

    virtual void Shutdown() override;

private:
    DArray<CPointLight*> _registeredPointLightComponents;

    CPointLight* _lastPointLightComponent = nullptr;
    DynamicGPUBuffer2<PointLight> _pointLightBuffer{};
};
