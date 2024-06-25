#include "ECS/Systems/LevelEditorSystem.h"
#include "Level.h"
#include "LevelEditorGame.h"
#include "PhysicsSystem.h"
#include "ECS/Components/CCamera.h"
#include "ECS/Components/CCollider.h"
#include "ECS/Components/CLevelEdit.h"
#include "ECS/Components/CRigidBody.h"
#include "ECS/Components/CStaticMesh.h"
#include "Engine/Subsystems/GameplaySubsystem.h"
#include "Engine/Subsystems/InputSubsystem.h"
#include "Rendering/Widgets/ViewportWidget.h"

LevelEditorSystem::LevelEditorSystem(const LevelEditorSystem& other) : System(other)
{
}

void LevelEditorSystem::SetLevel(const SharedObjectPtr<Level>& level)
{
    _level = level;

    _level->LoadAllChunks();
    _level->InitializeEdit();

    const AssetManager& assetManager = AssetManager::Get();

    const Level& constLevel = *_level;
    constLevel.ForEachChunk([&](const Level::Chunk& chunk)
    {
        for (const Level::EntityElement& element : chunk.EntityElements)
        {
            SpawnEntity(
                assetManager.FindAsset<EntityTemplate>(element.EntityTemplateID),
                element.EntityTransform,
                element.ID
            );
        }
    });
}

SharedObjectPtr<Level> LevelEditorSystem::GetLevel() const
{
    return _level;
}

void LevelEditorSystem::SpawnEntity(const SharedObjectPtr<EntityTemplate>& entityTemplate) const
{
    const Transform& cameraTransform = GetCamera()->GetTransform();

    Transform transform;
    transform.SetWorldLocation(cameraTransform.GetWorldLocation() + cameraTransform.GetForwardVector() * 5.0f);
    transform.SetParent(nullptr);

    SpawnEntity(entityTemplate, transform);
}

void LevelEditorSystem::SpawnEntity(const SharedObjectPtr<EntityTemplate>& entityTemplate, const Transform& transform,
                                    uint64 id /*= 0*/) const
{
    if (!entityTemplate->Load())
    {
        LOG(L"ERROR: Failed to load entity template {}.", entityTemplate->GetName());
        return;
    }

    if (id == 0)
    {
        id = _level->AddEntity(entityTemplate, transform);
    }

    // todo this breaks ptrs in physics body - we really need to implement event EntityArchetypeChanged
    GetWorld().CreateEntityAsync(
        entityTemplate,
        [id, transform, this](Entity& entity, const Archetype& archetype)
        {
            World& world = GetWorld();

            Entity* entityPtr = &entity;
            if (!archetype.HasComponent<CRigidBody>())
            {
                entityPtr = world.AddComponent<CRigidBody>(*entityPtr, Name(L"LevelEditorRigidBody")).NewEntity;
            }

            if (!archetype.HasComponent<CCollider>())
            {
                World::AddComponentResult<CCollider> result = world.AddComponent<CCollider>(
                    *entityPtr,
                    Name(L"LevelEditorCollider")
                );
                entityPtr = result.NewEntity;

                if (CStaticMesh* mesh = entity.GetChecked<CStaticMesh>(archetype))
                {
                    MeshCollision meshCollision;
                    meshCollision.Mesh = mesh->Mesh;
                    result.Component->Shape = meshCollision;
                }
            }

            World::AddComponentResult<CLevelEdit> result = world.AddComponent<CLevelEdit>(
                *entityPtr,
                Name(L"LevelEditorComponent")
            );
            entityPtr = result.NewEntity;
            result.Component->LevelElementID = id;

            world.Get<CTransform>(*entityPtr, archetype).ComponentTransform = transform;

            return entityPtr;
        },
        [](Entity& entity, const Archetype& archetype)
        {
        }
    );
}

void LevelEditorSystem::Initialize()
{
    System::Initialize();

    GameplaySubsystem& gameplaySubsystem = GameplaySubsystem::Get();
    SetLevel(gameplaySubsystem.GetGame<LevelEditorGame>()->GetLevel());

    SharedObjectPtr<ViewportWidget> viewport = gameplaySubsystem.GetMainViewport();
    std::ignore = viewport->OnPressed.Add([this, viewport]
    {
        const Vector3 direction = viewport->GetMouseDirectionWS();
        GetEventQueue().Enqueue([this, direction](SystemBase* system)
        {
            ProcessMouseClick(direction);
        });
    });

    InputSubsystem& inputSubsystem = InputSubsystem::Get();
    _onMoveSelectedHandle = inputSubsystem.GetKey(EKey::One).OnKeyUp.Add([this]()
    {
        _controlMode = EControlMode::Move;
    });

    _onRotateSelectedHandle = inputSubsystem.GetKey(EKey::Two).OnKeyUp.Add([this]()
    {
        _controlMode = EControlMode::Rotate;
    });

    _onScaleSelectedHandle = inputSubsystem.GetKey(EKey::Three).OnKeyUp.Add([this]()
    {
        _controlMode = EControlMode::Scale;
    });

    _onCoordinateSpaceChangedHandle = inputSubsystem.GetKey(EKey::Four).OnKeyUp.Add([this]()
    {
        _worldSpaceControls = !_worldSpaceControls;
    });

    _onArchetypeChangedHandle = GetWorld().OnArchetypeChanged.RegisterListener(_onArchetypeChanged);
}

void LevelEditorSystem::Tick(double deltaTime)
{
    GetEventQueue().ProcessEvents();

    if (_selectedEntity == nullptr)
    {
        return;
    }

    for (auto& entityListStruct : _onArchetypeChanged.GetEntityLists())
    {
        EventArchetypeChanged::EventData eventData;
        while (entityListStruct.Queue.Dequeue(eventData))
        {
            if (_selectedEntity == eventData.Entity)
            {
                SelectEntity(*std::get<Entity*>(eventData.Arguments));
            }
        }
    }

    CTransform& transform = Get<CTransform>(*_selectedEntity);

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

    if (moveDirection.LengthSquared() < 0.001f)
    {
        return;
    }


    switch (_controlMode)
    {
        case EControlMode::Move:
        {
            if (_worldSpaceControls)
            {
                location += moveDirection * static_cast<float>(deltaTime);
                transform.ComponentTransform.SetWorldLocation(location);
            }
            else
            {
                location += transform.ComponentTransform * moveDirection * static_cast<float>(deltaTime);
                transform.ComponentTransform.SetWorldLocation(location);
            }

            break;
        }
        case EControlMode::Rotate:
        {
            constexpr float rotationSpeed = 5.0f;

            if (_worldSpaceControls)
            {
                const Vector3 newRotation = transform.ComponentTransform.GetWorldRotationEuler() +
                    moveDirection * rotationSpeed * static_cast<float>(deltaTime);
                transform.ComponentTransform.SetWorldRotation(newRotation);
            }
            else
            {
                const Vector3 newRotation = transform.ComponentTransform.GetRelativeRotation() +
                    moveDirection * rotationSpeed * static_cast<float>(deltaTime);
                transform.ComponentTransform.SetRelativeRotation(newRotation);
            }

            break;
        }
        case EControlMode::Scale:
        {
            constexpr float scaleSpeed = 1.0f;

            if (_worldSpaceControls)
            {
                const Vector3 newScale = transform.ComponentTransform.GetWorldScale() +
                    moveDirection * scaleSpeed * static_cast<float>(deltaTime);
                transform.ComponentTransform.SetWorldScale(newScale);
            }
            else
            {
                const Vector3 newScale = transform.ComponentTransform.GetRelativeScale() +
                    moveDirection * scaleSpeed * static_cast<float>(deltaTime);
                transform.ComponentTransform.SetRelativeScale(newScale);
            }

            break;
        }
    }

    CLevelEdit& levelEdit = _selectedEntity->Get<CLevelEdit>(_selectedEntityArchetype);
    levelEdit.LevelElementID = _level->MoveEntity(levelEdit.LevelElementID, transform.ComponentTransform);
}

void LevelEditorSystem::OnEntityDestroyed(const Archetype& archetype, Entity& entity)
{
    System::OnEntityDestroyed(archetype, entity);

    if (_selectedEntity == &entity)
    {
        _selectedEntity = nullptr;
    }

    CLevelEdit& levelEdit = entity.Get<CLevelEdit>(archetype);
    _level->RemoveEntity(levelEdit.LevelElementID);
}

void LevelEditorSystem::Shutdown()
{
    System::Shutdown();

    InputSubsystem& inputSubsystem = InputSubsystem::Get();
    inputSubsystem.GetKey(EKey::One).OnKeyUp.Remove(_onMoveSelectedHandle);
    inputSubsystem.GetKey(EKey::Two).OnKeyUp.Remove(_onRotateSelectedHandle);
    inputSubsystem.GetKey(EKey::Three).OnKeyUp.Remove(_onScaleSelectedHandle);
    inputSubsystem.GetKey(EKey::Four).OnKeyUp.Remove(_onCoordinateSpaceChangedHandle);

    GetWorld().OnArchetypeChanged.UnregisterListener(_onArchetypeChangedHandle);
}

void LevelEditorSystem::ProcessMouseClick(const Vector3 direction)
{
    const Vector3 start = GetCamera()->GetTransform().GetWorldLocation();
    const Vector3 end = start + direction * 50.0f;

    PhysicsSystem::Hit hit = GetWorld().FindSystem<PhysicsSystem>()->Raycast(start, end);
    if (hit.IsValid)
    {
        SelectEntity(*hit.OtherBody->Entity);
    }
    else
    {
        _selectedEntity = nullptr;
    }
}

const CCamera* LevelEditorSystem::GetCamera() const
{
    return GameplaySubsystem::Get().GetMainViewport()->GetCamera();
}

void LevelEditorSystem::SelectEntity(Entity& entity)
{
    _selectedEntity = &entity;
    _selectedEntityArchetype = Archetype(*_selectedEntity);
    CacheArchetype(_selectedEntityArchetype);
}
