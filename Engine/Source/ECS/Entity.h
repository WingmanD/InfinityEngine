#pragma once

#include "Containers/DArray.h"
#include "ECS/Components/Component.h"
#include "ECS/Archetype.h"
#include "PassKey.h"

class World;

class Entity final : public IValidateable
{
public:
    friend class Archetype;
    friend class IValidateable;

public:
    explicit Entity() = default;
    explicit Entity(uint64_t id);

    [[nodiscard]] uint64_t GetID() const;

    void AddComponent(const SharedObjectPtr<Component>& newComponent, PassKey<World>);
    void RemoveComponent(uint16 index, PassKey<World>);

    Component* Get(uint16 index);
    
    /*
     * Get component at index.
     * NOTE: This does not check if component at index is of correct or if it exists.
     * You must use Archetype to get correct index. Archetype guarantees that component at index is of correct type
     * and that it exists.
     */
    template <typename ComponentType> requires IsA<ComponentType, Component>
    ComponentType* Get(uint16 index)
    {
        ComponentType* component = static_cast<ComponentType*>(Get(index));
        assert(component != nullptr);

        return component;
    }

    template <typename ComponentType> requires IsA<ComponentType, Component>
    ComponentType* Get(const Archetype& archetype)
    {
        uint16 index = archetype.GetComponentIndex<ComponentType>();
        return Get<ComponentType>(index);
    }

    void Destroy();

private:
    uint64_t _id = 0;

    // todo this should be unique ptr, but Object only returns shared ptr
    DArray<SharedObjectPtr<Component>, 5> _components{};

    // IValidateable
private:
    void SetValidImplementation(bool value);
    bool IsValidImplementation() const;
};
