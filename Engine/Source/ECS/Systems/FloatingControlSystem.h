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
    void TakeControlOf(Entity& entity);
    void ReleaseControl();

public:
    virtual void Initialize() override;
    virtual void Tick(double deltaTime) override;
    virtual void Shutdown() override;
    
private:
    // todo weak entity ptr
    Entity* _controlledEntity = nullptr;

    DelegateHandle _onMouseMovedHandle;

    Vector2 _mouseDelta;

private:
    void TakeControlOfInternal(Entity& entity);
    void ReleaseControlInternal();
};
