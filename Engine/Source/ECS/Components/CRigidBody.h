#pragma once

#include "Component.h"
#include "CRigidBody.reflection.h"

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
    
    uint32 BodyIndex = 0;
};
