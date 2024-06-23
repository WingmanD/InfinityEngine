#pragma once

#include "ECS/Systems/System.h"
#include "ECS/Components/CHealth.h"
#include "HealthSystem.reflection.h"

REFLECTED()
class HealthSystem : public System<CHealth>
{
    GENERATED()
    
public:
    HealthSystem() = default;
    HealthSystem(const HealthSystem& other);
    
    void DamageEntity(Entity& entity, const Archetype& archetype, float damage);
    
    // System
public:
    virtual void Tick(double deltaTime) override;

private:
    PROPERTY()
    Event<TypeSet<>, float /*Damage*/> _onEntityDamaged;
};
