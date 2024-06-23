#pragma once

#include "PhysicsSystem.h"
#include "ECS/Systems/System.h"
#include "ECS/Components/CProjectile.h"
#include "ECS/Components/CTransform.h"
#include "ProjectileSystem.reflection.h"

REFLECTED()
class ProjectileSystem : public System<const CProjectile, const CTransform, CRigidBody>
{
    GENERATED()
    
public:
    ProjectileSystem() = default;
    ProjectileSystem(const ProjectileSystem& other);

    // System
public:
    virtual void Initialize() override;
    virtual void OnEntityCreated(const Archetype& archetype, Entity& entity) override;
    virtual void Tick(double deltaTime) override;
    virtual void Shutdown() override;

private:
    PROPERTY()
    PhysicsSystem::EventHit _onHit;
    EventHandle _onHitHandle;
};
