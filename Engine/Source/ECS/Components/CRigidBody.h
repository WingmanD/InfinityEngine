#pragma once

#include "BoundingBox.h"
#include "Component.h"
#include "CRigidBody.reflection.h"

class Entity;
class CRigidBody;
class CTransform;
class CCollider;

struct Body
{
public:
    Entity* Entity;
    BoundingBox AABB;
    uint16 TransformIndex;
    uint16 RigidBodyIndex;
    uint16 ColliderIndex;
    DArray<uint32, 4> IndicesInCells;

public:
    CTransform& GetTransform() const;
    CRigidBody& GetRigidBody() const;
    CCollider& GetCollider() const;
};

REFLECTED()
enum class ERigidBodyState : uint8
{
    Static,
    Dormant,
    Dynamic
};

REFLECTED()
class CRigidBody : public Component
{
    GENERATED()

public:
    Vector3 Velocity = Vector3::Zero;

    PROPERTY(Edit, Serialize)
    float Mass = 1.0f;

    PROPERTY(Edit, Serialize)
    Vector3 CenterOfMass = Vector3::Zero;

    PROPERTY(Edit, Serialize)
    float Drag = 0.01f;

    Vector3 AngularVelocity = Vector3::Zero;

    PROPERTY(Edit, Serialize)
    float AngularDrag = 0.01f;

    PROPERTY(Edit, Serialize)
    ERigidBodyState State = ERigidBodyState::Static;

    PROPERTY(Edit, Serialize)
    float Restitution = 0.1f;

    PROPERTY(Edit, Serialize)
    float Inertia = 0.2f;
    
    Body PhysicsBody;
};
