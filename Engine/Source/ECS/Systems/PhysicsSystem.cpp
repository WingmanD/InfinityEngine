#include "PhysicsSystem.h"
#include "ECS/Components/CStaticMesh.h"
#include "Math/Math.h"

void PhysicsSystem::OnEntityCreated(const Archetype& archetype, Entity& entity)
{
    System::OnEntityCreated(archetype, entity);

    const uint32 transformIndex = archetype.GetComponentIndex<CTransform>();
    const uint32 rigidBodyIndex = archetype.GetComponentIndex<CRigidBody>();
    const uint32 colliderIndex = archetype.GetComponentIndex<CCollider>();

    const CStaticMesh* staticMesh = entity.GetChecked<CStaticMesh>(archetype);
    CTransform& transform = entity.Get<CTransform>(transformIndex);
    CRigidBody& rigidBody = entity.Get<CRigidBody>(rigidBodyIndex);
    CCollider& collider = entity.Get<CCollider>(colliderIndex);
    
    collider.ColliderTransform.SetParent(&transform.ComponentTransform);
    collider.Bounds = staticMesh->Mesh->GetBoundingBox();

    Body body;
    body.Entity = &entity;
    body.AABB = collider.Bounds.TransformBy(transform.ComponentTransform);
    body.TransformIndex = transformIndex;
    body.RigidBodyIndex = rigidBodyIndex;
    body.ColliderIndex = colliderIndex;
    
    GetCellAt(transform.ComponentTransform.GetWorldLocation()).AddBody(body, rigidBody.State);
}

void PhysicsSystem::Tick(double deltaTime)
{
    constexpr double substepDuration = 1.0f / 120.0f;

    double remainingTime = deltaTime;
    while (remainingTime > 0.001f)
    {
        const double substepTime = std::min(remainingTime, substepDuration);
        remainingTime -= substepTime;

        System::Tick(substepTime);

        NarrowPhase(substepTime);

        _narrowPhaseInputPairs.Clear();
    }
}

void PhysicsSystem::ProcessEntityList(EntityList& entityList, double deltaTime)
{
    System::ProcessEntityList(entityList, deltaTime);

    entityList.ForEach([this, deltaTime](Entity& entity)
    {
        CRigidBody& rigidBody = Get<CRigidBody>(entity);
        if (rigidBody.State != ERigidBodyState::Dynamic)
        {
            return true;
        }
        
        CTransform& transform = Get<CTransform>(entity);
        
        const Vector3 force = rigidBody.Mass * _gravity;
        const Vector3 acceleration = force / rigidBody.Mass;
        rigidBody.Velocity = rigidBody.Velocity + acceleration * static_cast<float>(deltaTime);

        const Vector3 currentLocation = transform.ComponentTransform.GetWorldLocation();
        const Vector3 newLocation = currentLocation + rigidBody.Velocity * static_cast<float>(deltaTime);

        if (GetWorld().WorldBounds.Contains(newLocation))
        {
            Move(rigidBody, currentLocation, newLocation, deltaTime);
        }
        else
        {
            // todo migrate entity to another world
            GetWorld().DestroyEntityAsync(entity);
        }
        
        return true;
    });
}

void PhysicsSystem::OnEntityDestroyed(const Archetype& archetype, Entity& entity)
{
    System::OnEntityDestroyed(archetype, entity);
    
    const CTransform& transform = entity.Get<const CTransform>(archetype);
    const CRigidBody& rigidBody = entity.Get<const CRigidBody>(archetype);

    GetCellAt(transform.ComponentTransform.GetWorldLocation()).RemoveBody(rigidBody.BodyIndex);
}

CTransform& PhysicsSystem::Body::GetTransform() const
{
    return Entity->Get<CTransform>(TransformIndex);
}

CRigidBody& PhysicsSystem::Body::GetRigidBody() const
{
    return Entity->Get<CRigidBody>(RigidBodyIndex);
}

CCollider& PhysicsSystem::Body::GetCollider() const
{
    return Entity->Get<CCollider>(ColliderIndex);
}

void PhysicsSystem::Cell::AddBody(const Body& body, ERigidBodyState bodyType)
{
    Bodies.Resize(StaticBodyCount + DormantBodyCount + DynamicBodyCount + 1);
    
    switch (bodyType)
    {
        case ERigidBodyState::Static:
        {
            body.GetRigidBody().BodyIndex = StaticBodyCount;
            
            const uint32 firstDynamicIndex = StaticBodyCount + DormantBodyCount;
            const uint32 lastDynamicIndex = firstDynamicIndex + DynamicBodyCount;

            if (DynamicBodyCount > 0)
            {
                const Body& firstDynamicBody = Bodies[firstDynamicIndex];
                firstDynamicBody.GetRigidBody().BodyIndex = lastDynamicIndex;
                Bodies[lastDynamicIndex] = firstDynamicBody;
            }

            if (DormantBodyCount > 0)
            {
                Body& firstDormantBody = Bodies[StaticBodyCount];
                firstDormantBody.GetRigidBody().BodyIndex = firstDynamicIndex;
                
                Bodies[firstDynamicIndex] = firstDormantBody;
                firstDormantBody = body;
            }
            else
            {
                Bodies[StaticBodyCount] = body;
            }
            
            ++StaticBodyCount;
            
            break;
        }

        case ERigidBodyState::Dormant:
        {
            const uint32 firstDynamicIndex = StaticBodyCount + DormantBodyCount;
            const uint32 lastDynamicIndex = firstDynamicIndex + DynamicBodyCount;

            Body& firstDynamicBody = Bodies[firstDynamicIndex];
            if (DynamicBodyCount > 0)
            {
                firstDynamicBody.GetRigidBody().BodyIndex = lastDynamicIndex;
                Bodies[lastDynamicIndex] = firstDynamicBody;
            }

            body.GetRigidBody().BodyIndex = firstDynamicIndex;
            firstDynamicBody = body;
    
            ++DormantBodyCount;
            break;
        }
        
        case ERigidBodyState::Dynamic:
        {
            const uint32 index = StaticBodyCount + DormantBodyCount + DynamicBodyCount;
            body.GetRigidBody().BodyIndex = index;
            
            Bodies[index] = body;
            
            ++DynamicBodyCount;
            break;
        }
    }
}

void PhysicsSystem::Cell::RemoveBody(uint32 index)
{
    if (index >= StaticBodyCount + DormantBodyCount)
    {
        if (StaticBodyCount + DormantBodyCount > 0)
        {
            const uint32 lastDynamicIndex = StaticBodyCount + DormantBodyCount + DynamicBodyCount - 1;
            if (index < lastDynamicIndex)
            {
                const Body& lastDynamicBody = Bodies[lastDynamicIndex];
                lastDynamicBody.GetRigidBody().BodyIndex = index;
                Bodies[index] = lastDynamicBody;
            }
        }

        --DynamicBodyCount;
    }
    else if (index >= StaticBodyCount)
    {
        const uint32 lastDormantIndex = StaticBodyCount + DormantBodyCount - 1;
        Body& lastDormantBody = Bodies[lastDormantIndex];
        lastDormantBody.GetRigidBody().BodyIndex = index;
        Bodies[index] = lastDormantBody;

        const Body& lastDynamicBody = Bodies[StaticBodyCount + DormantBodyCount + DynamicBodyCount - 1];
        lastDynamicBody.GetRigidBody().BodyIndex = lastDormantIndex;
        lastDormantBody = lastDynamicBody;

        --DormantBodyCount;
    }
    else
    {
        const uint32 lastStaticIndex = StaticBodyCount - 1;

        Body& lastStaticBody = Bodies[lastStaticIndex];
        lastStaticBody.GetRigidBody().BodyIndex = index;
        Bodies[index] = lastStaticBody;

        Body& lastDormantBody = Bodies[lastStaticIndex + DormantBodyCount];
        lastDormantBody.GetRigidBody().BodyIndex = lastStaticIndex;
        lastStaticBody = lastDormantBody;

        const Body& lastDynamicBody = Bodies[StaticBodyCount + DormantBodyCount + DynamicBodyCount - 1];
        lastDynamicBody.GetRigidBody().BodyIndex = lastStaticIndex + DormantBodyCount;
        lastDormantBody = lastDynamicBody;

        StaticBodyCount--;
    }
}

void PhysicsSystem::Cell::SetBodyState(uint32 index, ERigidBodyState bodyType)
{
}

PhysicsSystem::Cell& PhysicsSystem::GetCellAt(const Vector3& location)
{
    const BoundingBox& worldBounds = GetWorld().WorldBounds;
    const Vector3 relativeLocation = location - worldBounds.GetMin();

    const Vector3 index3D = relativeLocation / (worldBounds.GetExtent() * 2.0f / _cellCountX);
    const uint32 x = static_cast<uint32>(index3D.x);
    const uint32 y = static_cast<uint32>(index3D.y);
    const uint32 z = static_cast<uint32>(index3D.z);

    const uint32 index = x + y * _cellCountX + z * _cellCountX * _cellCountX;

    return _cells[index];
}

void PhysicsSystem::Move(const CRigidBody& rigidBody, const Vector3& currentLocation, const Vector3& newLocation, double deltaTime)
{
    Cell& oldCell = GetCellAt(currentLocation);
    Body& body = oldCell.Bodies[rigidBody.BodyIndex];

    Vector3 sweepDirection = newLocation - currentLocation;
    const float distance = sweepDirection.Length();
    sweepDirection.Normalize();
    
    const float sweepDistance = body.AABB.SweepDistance(sweepDirection);
    
    float distanceRemaining = distance;
    
    BoundingBox lastAABB = body.AABB;
    while (distanceRemaining > 0.0f)
    {
        BoundingBox sweepAABB = body.AABB;

        const float distancePassed = Math::Min(sweepDistance, distanceRemaining);
        distanceRemaining -= distancePassed;

        const Vector3 sweepVector = sweepDirection * distancePassed;

        body.AABB.Move(sweepVector);
        sweepAABB = sweepAABB.Union(body.AABB);

        Cell& cellAtSweep = GetCellAt(sweepAABB.GetCenter());

        bool hitFound = false;
        for (Body& cellBody : cellAtSweep.Bodies)
        {
            if (cellBody.Entity == body.Entity)
            {
                continue;
            }
        
            if (sweepAABB.Overlap(cellBody.AABB))
            {
                if (CollisionCheck(body, cellBody).IsValid)
                {
                    _narrowPhaseInputPairs.Add({&body, &cellAtSweep.Bodies[0]});
                    
                    hitFound = true;
                    break;
                }
            }
        }
        
        if (!hitFound)
        {
            lastAABB = body.AABB;
        }
        else
        {
            break;
        }
    }

    const Vector3 location = lastAABB.GetCenter();
    Cell& newCell = GetCellAt(location);

    Transform& transform = body.GetTransform().ComponentTransform;
    transform.SetWorldLocation(location);

    const Vector3 deltaRotationRad = rigidBody.AngularVelocity * static_cast<float>(deltaTime);
    transform.SetWorldRotation(
        transform.GetWorldRotationEuler() + Math::ToDegrees(deltaRotationRad)
    );

    Move(rigidBody, oldCell, newCell);
}

void PhysicsSystem::Move(const CRigidBody& rigidBody, Cell& currentCell, Cell& newCell)
{
    Body& body = currentCell.Bodies[rigidBody.BodyIndex];

    CCollider& collider = body.Entity->Get<CCollider>(body.ColliderIndex);
    body.AABB = collider.Bounds.TransformBy(body.Entity->Get<CTransform>(body.TransformIndex).ComponentTransform);

    if (&currentCell != &newCell)
    {
        const uint32 oldIndex = rigidBody.BodyIndex;
        newCell.AddBody(body, rigidBody.State);
        currentCell.RemoveBody(oldIndex);
    }
}

void PhysicsSystem::BroadPhase(Body& body, Cell& cell)
{
    for (Body& cellBody : cell.Bodies)
    {
        if (cellBody.Entity == body.Entity)
        {
            continue;
        }
        
        if (body.AABB.Overlap(cellBody.AABB))
        {
            _narrowPhaseInputPairs.Add({&body, &cellBody});
        }
    }
}

void PhysicsSystem::NarrowPhase(double deltaTime)
{
    for (const CollisionPair& pair : _narrowPhaseInputPairs)
    {
        CRigidBody& rigidBodyA = pair.BodyA->GetRigidBody();
        CRigidBody& rigidBodyB = pair.BodyB->GetRigidBody();

        const Vector3 velocityA = rigidBodyA.Velocity;
        const Vector3 velocityB = rigidBodyB.Velocity;

        const Vector3 relativeVelocity = velocityA - velocityB;
        const Vector3 normal = pair.BodyA->AABB.GetCenter() - pair.BodyB->AABB.GetCenter();
        const float distance = normal.Length();
        const float penetration = pair.BodyA->AABB.GetExtent().Length() + pair.BodyB->AABB.GetExtent().Length() - distance;

        if (penetration > 0.0f)
        {
            const float restitution = 0.5f * (rigidBodyA.Restitution + rigidBodyB.Restitution);
            const float impulse = -(1.0f + restitution) * relativeVelocity.Dot(normal) /
                (1.0f / rigidBodyA.Mass + (rigidBodyB.State != ERigidBodyState::Static ? 1.0f / rigidBodyB.Mass : 0.0f));

            const Vector3 impulseVector = impulse * normal;

            {
                rigidBodyA.Velocity += impulseVector / rigidBodyA.Mass;
                
                CTransform& transformA = pair.BodyA->Entity->Get<CTransform>(pair.BodyA->TransformIndex);
                const Vector3 currentLocation = transformA.ComponentTransform.GetWorldLocation();
                const Vector3 newLocation = currentLocation + rigidBodyA.Velocity * static_cast<float>(deltaTime);
                transformA.ComponentTransform.SetWorldLocation(newLocation);

                // todo move adds to _narrowPhaseInputPairs, during iteration
                Move(rigidBodyA, currentLocation, newLocation, deltaTime);
            }

            // RigidBody A is always dynamic, but B can be anything
            if (rigidBodyB.State == ERigidBodyState::Dynamic)
            {
                rigidBodyB.Velocity -= impulseVector / rigidBodyB.Mass;
                
                CTransform& transformB = pair.BodyA->Entity->Get<CTransform>(pair.BodyA->TransformIndex);
                const Vector3 currentLocation = transformB.ComponentTransform.GetWorldLocation();
                const Vector3 newLocation = transformB.ComponentTransform.GetWorldLocation() + rigidBodyA.Velocity * static_cast<float>(deltaTime);
                transformB.ComponentTransform.SetWorldLocation(newLocation);

                Move(rigidBodyB, currentLocation, newLocation, deltaTime);
            }
        }
    }
}

PhysicsSystem::Hit PhysicsSystem::CollisionCheck(const Body& bodyA, const Body& bodyB)
{
    CRigidBody& rigidBodyA = bodyA.GetRigidBody();
    CRigidBody& rigidBodyB = bodyB.GetRigidBody();

    CCollider& colliderA = bodyA.GetCollider();

    const Vector3 velocityA = rigidBodyA.Velocity;
    const Vector3 velocityB = rigidBodyB.Velocity;

    const Vector3 relativeVelocity = velocityA - velocityB;
    const Vector3 normal = bodyA.AABB.GetCenter() - bodyB.AABB.GetCenter();
    const float distance = normal.Length();
    const float penetration = bodyA.AABB.GetExtent().Length() + bodyB.AABB.GetExtent().Length() - distance;

    Hit hit;
    hit.IsValid = true;

    return hit;
}
