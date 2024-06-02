#include "FloatingControlSystem.h"

#include "Engine/Subsystems/GameplaySubsystem.h"
#include "Engine/Subsystems/InputSubsystem.h"
#include "Math/Math.h"
#include "Rendering/Widgets/ViewportWidget.h"

void FloatingControlSystem::TakeControlOf(Entity& entity)
{
    GetEventQueue().Enqueue([this, &entity](SystemBase* system)
    {
        TakeControlOfInternal(entity);
    });
}

void FloatingControlSystem::ReleaseControl()
{
    GetEventQueue().Enqueue([this](SystemBase* system)
    {
        ReleaseControl();
    });
}

void FloatingControlSystem::Initialize()
{
    System::Initialize();

    _onMouseMovedHandle = InputSubsystem::Get().OnMouseMoved.Add([this](const Vector2& delta)
    {
        Vector2 newDelta = delta;
        newDelta.x = -newDelta.x;
        
        _mouseDelta += newDelta;
    });
}

void FloatingControlSystem::Tick(double deltaTime)
{
    GetEventQueue().ProcessEvents();
    
    if (_controlledEntity == nullptr)
    {
        return;
    }

    if (!GameplaySubsystem::Get().GetMainViewport()->IsFocused())
    {
        return;
    }

    const CFloatingControl& control = Get<const CFloatingControl>(*_controlledEntity);
    CTransform& transform = Get<CTransform>(*_controlledEntity);

    const Vector2 mouseDelta = _mouseDelta * control.AngularSpeed * static_cast<float>(deltaTime);

    Vector3 eulerAngles = transform.ComponentTransform.GetWorldRotationEuler();
    eulerAngles.x = 0.0f;
    eulerAngles.y = Math::Clamp(eulerAngles.y - mouseDelta.y, -89.0f, 89.0f);
    eulerAngles.z += mouseDelta.x;

    const Quaternion rotation = Math::MakeQuaternionFromEuler(eulerAngles);
    transform.ComponentTransform.SetWorldRotation(rotation);

    _mouseDelta = Vector2::Zero;

    Vector3 location = transform.ComponentTransform.GetWorldLocation();
    const Vector3 forward = transform.ComponentTransform.GetForwardVector();
    const Vector3 right = transform.ComponentTransform.GetRightVector();
    const Vector3 up = transform.ComponentTransform.GetUpVector();

    const InputSubsystem& inputSubsystem = InputSubsystem::Get();

    Vector3 moveDirection = Vector3::Zero;
    if (inputSubsystem.IsKeyDown(EKey::W))
    {
        moveDirection += forward;
    }
    if (inputSubsystem.IsKeyDown(EKey::S))
    {
        moveDirection -= forward;
    }
    if (inputSubsystem.IsKeyDown(EKey::A))
    {
        moveDirection -= right;
    }
    if (inputSubsystem.IsKeyDown(EKey::D))
    {
        moveDirection += right;
    }
    if (inputSubsystem.IsKeyDown(EKey::Q))
    {
        moveDirection += up;
    }
    if (inputSubsystem.IsKeyDown(EKey::E))
    {
        moveDirection -= up;
    }

    location += moveDirection * control.Speed * static_cast<float>(deltaTime);

    transform.ComponentTransform.SetWorldLocation(location);
    
    GetEventQueue().ProcessEvents();
}

void FloatingControlSystem::Shutdown()
{
    System::Shutdown();

    InputSubsystem::Get().OnMouseMoved.Remove(_onMouseMovedHandle);
}

void FloatingControlSystem::TakeControlOfInternal(Entity& entity)
{
    ReleaseControlInternal();

    _controlledEntity = &entity;
    CacheArchetype(Archetype(entity));
}

void FloatingControlSystem::ReleaseControlInternal()
{
    _controlledEntity = nullptr;
}
