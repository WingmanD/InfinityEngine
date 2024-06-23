#pragma once

#include "System.h"
#include "ECS/Components/CTransform.h"
#include "ECS/Components/CCamera.h"
#include "LevelEditorSystem.reflection.h"

class Level;

REFLECTED()
class LevelEditorSystem : public System<CTransform>
{
    GENERATED()

public:
    LevelEditorSystem() = default;
    LevelEditorSystem(const LevelEditorSystem& other);
    
    void SetLevel(const SharedObjectPtr<Level>& level);
    SharedObjectPtr<Level> GetLevel() const;

    void SpawnEntity(const SharedObjectPtr<EntityTemplate>& entityTemplate) const;
    void SpawnEntity(const SharedObjectPtr<EntityTemplate>& entityTemplate, const Transform& transform, uint64 id = 0u) const;
    
    // System
public:
    virtual void Initialize() override;
    virtual void Tick(double deltaTime) override;
    virtual void OnEntityDestroyed(const Archetype& archetype, Entity& entity) override;
    virtual void Shutdown() override;

private:
    enum class EControlMode
    {
        Move,
        Rotate,
        Scale
    };
    
    bool _worldSpaceControls = true;
    EControlMode _controlMode = EControlMode::Move;

    DelegateHandle _onMoveSelectedHandle;
    DelegateHandle _onRotateSelectedHandle;
    DelegateHandle _onScaleSelectedHandle;
    DelegateHandle _onCoordinateSpaceChangedHandle;

    PROPERTY()
    EventArchetypeChanged _onArchetypeChanged;
    EventHandle _onArchetypeChangedHandle;
    
    SharedObjectPtr<Level> _level;
    Entity* _selectedEntity = nullptr;
    Archetype _selectedEntityArchetype;

private:
    void ProcessMouseClick(const Vector3 direction);
    const CCamera* GetCamera() const;
    void SelectEntity(Entity& entity);
};
