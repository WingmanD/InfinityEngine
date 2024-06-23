#pragma once

#include "System.h"
#include "ECS/Components/CFloatingControl.h"
#include "ECS/Components/CTransform.h"
#include "ECS/Systems/FloatingControlSystem.reflection.h"

REFLECTED()
class FloatingControlSystem : public System<CFloatingControl, CTransform>
{
    GENERATED()

public:
    FloatingControlSystem() = default;
    FloatingControlSystem(const FloatingControlSystem& other);

    void TakeControlOf(Entity& entity);
    void ReleaseControl();

public:
    virtual void Initialize() override;
    virtual void Tick(double deltaTime) override;
    virtual void Shutdown() override;

private:
    // todo weak entity ptr
    Entity* _controlledEntity = nullptr;
    Archetype _controlledEntityArchetype;

    DelegateHandle _onMouseMovedHandle;

    Vector2 _mouseDelta;

    PROPERTY()
    EventArchetypeChanged _onArchetypeChanged;
    EventHandle _onArchetypeChangedHandle;

private:
    void TakeControlOfInternal(Entity& entity);
    void ReleaseControlInternal();
};
