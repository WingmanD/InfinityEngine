#include "CRigidBody.h"
#include "ECS/Entity.h"
#include "ECS/Components/CTransform.h"
#include "ECS/Components/CCollider.h"

CTransform& Body::GetTransform() const
{
    return Entity->Get<CTransform>(TransformIndex);
}

CRigidBody& Body::GetRigidBody() const
{
    return Entity->Get<CRigidBody>(RigidBodyIndex);
}

CCollider& Body::GetCollider() const
{
    return Entity->Get<CCollider>(ColliderIndex);
}
