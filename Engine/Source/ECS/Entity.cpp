#include "Entity.h"

Entity::Entity(uint64_t id) : _id(id)
{
}

uint64_t Entity::GetID() const
{
    return _id;
}

void Entity::AddComponent(const SharedObjectPtr<Component>& newComponent, PassKey<World>)
{
    _components.Emplace(newComponent);
}

void Entity::RemoveComponent(uint16 index, PassKey<World>)
{
    _components.RemoveAtSwap(index);
}

void Entity::Destroy()
{
    _id = 0;
    _components.Clear();
}

void Entity::SetValidImplementation(bool value)
{
    _id = value;
}

bool Entity::IsValidImplementation() const
{
    return _id != 0;
}
