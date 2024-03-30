#pragma once

#include "System.h"
#include "ECS/Components/CCamera.h"
#include "ECS/Components/CTransform.h"

class CameraSystem : public System<const CTransform, const CCamera>
{
protected:
    virtual void OnEntityCreated(const Archetype& archetype, Entity& entity) override;
};
