#pragma once

#include "ECS/Components/Component.h"
#include "ECS/EntityList.h"
#include "ECS/EntityListGraph.h"
#include "Containers/ObjectTypeMap.h"

class GameplaySubsystem;
class Type;
class Entity;

class World : public IValidateable
{
public:
    friend class IValidateable;
    
public:
    explicit World() = default;
    
    Entity& CreateEntity(const Archetype& archetype);
    Entity& RegisterEntity(const Archetype& archetype, Entity&& entity);
    
    void DestroyEntity(const Archetype& archetype,Entity& entity);

    // todo overloads taking Name - Name contains only uint64 that is hash of the string, store it in a registry for ToString
    SharedObjectPtr<Component> AddComponent(Entity& entity, Type& componentType);

    template <typename ComponentType> requires IsA<ComponentType, Component>
    SharedObjectPtr<Component> AddComponent(Entity& entity)
    {
        return AddComponent(entity, *ComponentType::StaticType());
    }

    void RemoveComponent(Entity& entity, uint16 index);

    void Tick(double deltaTime, PassKey<GameplaySubsystem>);

private:
    ObjectTypeMap _componentTypeMap;

    EntityListGraph _entityListGraph;

    bool _isValid = false;

        // IValidateable
private:
    void SetValidImplementation(bool valid);
    bool IsValidImplementation() const;
};
