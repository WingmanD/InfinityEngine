#include "FloatingControlSystem.h"
#include "Engine/Subsystems/GameplaySubsystem.h"
#include "Engine/Subsystems/InputSubsystem.h"
#include "Math/Math.h"
#include "Rendering/Widgets/ViewportWidget.h"

FloatingControlSystem::FloatingControlSystem(const FloatingControlSystem& other) : System(other)
{
}

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
        if (InputSubsystem::Get().IsMouseCaptured())
        {
            Vector2 newDelta = delta;
            newDelta.x = -newDelta.x;

            _mouseDelta += newDelta;
        }
    });

    _onArchetypeChangedHandle = GetWorld().OnArchetypeChanged.RegisterListener(_onArchetypeChanged);
}

void FloatingControlSystem::Tick(double deltaTime)
{
    GetEventQueue().ProcessEvents();

    if (_controlledEntity == nullptr)
    {
        return;
    }

    if (!_controlledEntity->IsValid())
    {
        for (auto& entityListStruct : _onArchetypeChanged.GetEntityLists())
        {
            EventArchetypeChanged::EventData eventData;
            while (entityListStruct.Queue.Dequeue(eventData))
            {
                if (_controlledEntity == eventData.Entity)
                {
                    TakeControlOf(*std::get<Entity*>(eventData.Arguments));
                    break;
                }
            }
        }
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

    GetWorld().OnArchetypeChanged.UnregisterListener(_onArchetypeChangedHandle);
}

void FloatingControlSystem::TakeControlOfInternal(Entity& entity)
{
    ReleaseControlInternal();

    _controlledEntity = &entity;
    _controlledEntityArchetype = Archetype(entity);
    CacheArchetype(_controlledEntityArchetype);
}

void FloatingControlSystem::ReleaseControlInternal()
{
    _controlledEntity = nullptr;
}
