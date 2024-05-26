#include "PhysicsSystem.h"

#include <queue>

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
    const CRigidBody& rigidBody = entity.Get<CRigidBody>(rigidBodyIndex);
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
    // Update overlaps
    for (Event<TypeSet<CTransform>>::EntityListStruct& entityListStruct : GetWorld().OnTransformChanged.GetEntityLists())
    {
        const uint16 rigidBodyIndex = entityListStruct.EntityArchetype.GetComponentIndexChecked<CRigidBody>();
        if (rigidBodyIndex == std::numeric_limits<uint16>::max())
        {
            continue;
        }

        entityListStruct.Update();

        for (const auto& eventData : entityListStruct.EntityListOutput)
        {
            if (!eventData.Entity->IsValid())
            {
                continue;
            }
            
            const CRigidBody& rigidBody = eventData.Entity->Get<CRigidBody>(rigidBodyIndex);

            // todo
            // BroadPhase()
        }
    }
    
    constexpr double substepDuration = 1.0f / 120.0f;

    double remainingTime = deltaTime;
    while (remainingTime > 0.001f)
    {
        const double substepTime = std::min(remainingTime, substepDuration);
        remainingTime -= substepTime;

        System::Tick(substepTime);
        
        NarrowPhase();

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

        const CTransform& transform = Get<CTransform>(entity);
        
        const Vector3 force = rigidBody.Mass * _gravity;
        const Vector3 acceleration = force / rigidBody.Mass;
        rigidBody.Velocity += acceleration * static_cast<float>(deltaTime);
        //LOG(L"Velocity after tick: {}", rigidBody.Velocity);

        const Vector3 currentLocation = transform.ComponentTransform.GetWorldLocation();
        const Vector3 newLocation = currentLocation + rigidBody.Velocity * static_cast<float>(deltaTime);

       //LOG(L"Desired location delta: {}", newLocation - currentLocation);

        if (GetWorld().WorldBounds.Contains(newLocation))
        {
            //LOG(L"Location before move: {}", transform.ComponentTransform.GetWorldLocation());
            Move(rigidBody, currentLocation, newLocation, deltaTime);
            //LOG(L"Location after move: {}", transform.ComponentTransform.GetWorldLocation());
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

    const Vector3 offset = body.GetTransform().ComponentTransform.GetWorldLocation() - body.AABB.GetCenter();
    
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
                Hit hit = CollisionCheck(body, cellBody, sweepDirection);
                if (hit.IsValid)
                {
                    ProcessHit(body, hit);
                    const Vector3 location = body.AABB.GetCenter();
                    body.AABB.Move(hit.ImpactNormal * hit.PenetrationDepth);
                    LOG(L"Resolving penetration: normal {}, depth {}, location before {}, location after {}",
                        hit.ImpactNormal,
                        hit.PenetrationDepth,
                        location,
                        body.AABB.GetCenter()
                    );

                    _narrowPhaseInputPairs.Add({&body, &cellAtSweep.Bodies[0]});

                    hitFound = true;
                    break;
                }
            }
        }
        
        if (hitFound)
        {
            break;
        }
    }

    const Vector3 location = body.AABB.GetCenter() + offset;
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

    const CCollider& collider = body.Entity->Get<CCollider>(body.ColliderIndex);
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

void PhysicsSystem::NarrowPhase()
{
    for (const CollisionPair& pair : _narrowPhaseInputPairs)
    {
        CRigidBody& rigidBodyA = pair.BodyA->GetRigidBody();
        CRigidBody& rigidBodyB = pair.BodyB->GetRigidBody();

        const Vector3 velocityA = rigidBodyA.Velocity;
        const Vector3 velocityB = rigidBodyB.Velocity;

        Hit hit = CollisionCheck(*pair.BodyA, *pair.BodyB);

        // todo update overlaps
    }
}

PhysicsSystem::Hit PhysicsSystem::CollisionCheck(const Body& bodyA, const Body& bodyB, const Vector3& movementDirection)
{
    Hit hit = CollisionCheck(bodyA, bodyB);

    LOG(L"Hit: location {}, normal {}, depth {}", hit.Location, hit.ImpactNormal, hit.PenetrationDepth);
    return hit;
}

PhysicsSystem::Hit PhysicsSystem::CollisionCheck(const Body& bodyA, const Body& bodyB)
{
    return GilbertJohnsonKeerthi(bodyA, bodyB);
}

void PhysicsSystem::ProcessHit(const Body& bodyA, const Hit& hit)
{
    CRigidBody& rigidBodyA = bodyA.GetRigidBody();
    CRigidBody& rigidBodyB = hit.OtherBody->GetRigidBody();

    CTransform& transformA = bodyA.GetTransform();

    const float mass = 1.0f / ((1.0f / rigidBodyA.Mass) + (1.0f / rigidBodyB.Mass));

    const Vector3 rA = hit.Location - transformA.ComponentTransform.GetWorldLocation();
    const Vector3 rB = hit.Location - hit.OtherBody->GetTransform().ComponentTransform.GetWorldLocation();

    const Vector3 relativeVelocity = (rigidBodyA.Velocity - rigidBodyB.Velocity) +
        rigidBodyA.AngularVelocity.Cross(rA) -
        rigidBodyB.AngularVelocity.Cross(rB);
    const float impulseSpeed = hit.ImpactNormal.Dot(relativeVelocity);

    const float restitution = (rigidBodyA.Restitution + rigidBodyB.Restitution) / 2.0f;
    const Vector3 impulse = hit.ImpactNormal * (1.0f + restitution) * impulseSpeed /
    (mass +
        rA.Cross(hit.ImpactNormal).Dot(rA.Cross(hit.ImpactNormal)) / rigidBodyA.Inertia +
        rB.Cross(hit.ImpactNormal).Dot(rB.Cross(hit.ImpactNormal)) / rigidBodyB.Inertia);

    //const Vector3 impulse = (1.0f + restitution) * impulseSpeed * hit.ImpactNormal / mass;

    rigidBodyA.Velocity -= impulse / rigidBodyA.Mass;
    rigidBodyA.AngularVelocity -= rA.Cross(impulse) / rigidBodyA.Inertia;
    
    const Vector3 velocityDeltaB = impulse * hit.ImpactNormal / rigidBodyB.Mass;
    const Vector3 angularVelocityDeltaB = rB.Cross(impulse) / rigidBodyB.Inertia;

    if (rigidBodyB.State != ERigidBodyState::Dynamic)
    {
        rigidBodyA.Velocity -= velocityDeltaB;
        rigidBodyA.AngularVelocity -= angularVelocityDeltaB;
    }
    else
    {
        rigidBodyB.Velocity += velocityDeltaB;
        rigidBodyB.AngularVelocity += angularVelocityDeltaB;
    }

    LOG(L"Velocity after hit: {}", rigidBodyA.Velocity);
}

PhysicsSystem::Hit PhysicsSystem::GilbertJohnsonKeerthi(const Body& bodyA, const Body& bodyB)
{
    Vector3 difference = MinkowskiDifference(bodyA, bodyB, Vector3::UnitX);
    DArray<Vector3, 4> simplex = {difference};

    Vector3 direction = -difference;
    while (true)
    {
        difference = MinkowskiDifference(bodyA, bodyB, direction);
        if (difference.Dot(direction) <= 0.0f)
        {
            LOG(L"Hit not found");
            return {};
        }

        simplex.InsertAt(0, difference);

        if (GJKSimplexContainsOrigin(simplex, direction))
        {
            LOG(L"Hit found");
            if (simplex.Count() == 4)
            {
                return ExpandingPolytopeAlgorithm(bodyA, bodyB, simplex);
            }

            // todo support for dot, line and triangle simplex expansion to tetrahedron
            DEBUG_BREAK();
            return {};
        }
    }
}

Vector3 PhysicsSystem::MinkowskiDifference(const Body& bodyA, const Body& bodyB, const Vector3& direction)
{
    Vector3 directionNormalized = direction;
    directionNormalized.Normalize();
    
    Vector3 furthestPointA;
    Vector3 transformedDirectionA = Vector3::TransformNormal(directionNormalized, bodyA.GetTransform().ComponentTransform.GetWorldMatrix().Invert());
    transformedDirectionA.Normalize();
    std::visit([&transformedDirectionA, &furthestPointA](auto&& arg)
    {
        furthestPointA = arg.FurthestPointInDirection(transformedDirectionA);
    },
    bodyA.GetCollider().Shape);

    furthestPointA = bodyA.GetTransform().ComponentTransform * furthestPointA;

    Vector3 furthestPointB;
    Vector3 transformedDirectionB = Vector3::TransformNormal(-directionNormalized, bodyB.GetTransform().ComponentTransform.GetWorldMatrix().Invert());
    transformedDirectionB.Normalize();
    std::visit([&transformedDirectionB, &furthestPointB](auto&& arg)
    {
        furthestPointB = arg.FurthestPointInDirection(transformedDirectionB);
    },
    bodyB.GetCollider().Shape);

    furthestPointB = bodyB.GetTransform().ComponentTransform * furthestPointB;

    return furthestPointA - furthestPointB;
}


bool PhysicsSystem::GJKSimplexContainsOrigin(DArray<Vector3, 4>& simplex, Vector3& direction) const
{
    switch (simplex.Count())
    {
        // Line, origin is above the line
        case 2:
        {
            return GJKLine(simplex, direction);
        }

        // Triangle contains origin
        case 3:
        {
            return GJKTriangle(simplex, direction);
        }

        // Tetrahedron contains origins
        case 4:
        {
            const Vector3 a = simplex[0];
            const Vector3 b = simplex[1];
            const Vector3 c = simplex[2];
            const Vector3 d = simplex[3];

            const Vector3 ab = b - a;
            const Vector3 ac = c - a;

            const Vector3 aOrigin = -a;
            
            const Vector3 abc = ab.Cross(ac);
            if (abc.Dot(aOrigin) > 0.0f)
            {
                simplex = {a, b, c};
                return GJKTriangle(simplex, direction);
            }

            const Vector3 ad = d - a;
            const Vector3 acd = ac.Cross(ad);
            if (acd.Dot(aOrigin) > 0.0f)
            {
                simplex = {a, c, d};
                return GJKTriangle(simplex, direction);
            }

            
            const Vector3 adb = ad.Cross(ab);
            if (adb.Dot(aOrigin) > 0.0f)
            {
                simplex = {a, d, b};
                return GJKTriangle(simplex, direction);
            }

            return true;
        }

        default:
        {
            DEBUG_BREAK();
            break;
        }
    }

    return false;
}

bool PhysicsSystem::GJKLine(DArray<Vector3, 4>& simplex, Vector3& direction) const
{
    const Vector3 a = simplex[0];
    const Vector3 b = simplex[1];

    const Vector3 ab = b - a;
    const Vector3 aOrigin = -a;

    if (ab.Dot(aOrigin) > 0.0f)
    {
        direction = ab.Cross(aOrigin).Cross(ab);
    }
    else
    {
        direction = aOrigin;
        simplex = {a};
    }

    return false;
}

bool PhysicsSystem::GJKTriangle(DArray<Vector3, 4>& simplex, Vector3& direction) const
{
    const Vector3 a = simplex[0];
    const Vector3 b = simplex[1];
    const Vector3 c = simplex[2];

    const Vector3 ab = b - a;
    const Vector3 ac = c - a;
    const Vector3 up = ab.Cross(ac);

    const Vector3 aOrigin = -a;

    if (up.Cross(ac).Dot(aOrigin) > 0.0f)
    {
        if (ac.Dot(aOrigin) > 0.0f)
        {
            simplex = {a, c};
            direction = ac.Cross(aOrigin).Cross(ac);
        }
        else
        {
            simplex = {a, b};
            return GJKLine(simplex, direction);
        }
    }
    else
    {
        if (ab.Cross(up).Dot(aOrigin) > 0.0f)
        {
            simplex = {a, b};
            return GJKLine(simplex, direction);
        }

        if (up.Dot(aOrigin) > 0.0f)
        {
            direction = up;
        }
        else
        {
            simplex = {a, c, b};
            direction = -up;
        }
    }
    return false;
}

bool PhysicsSystem::TetrahedronContainsOrigin(const DArray<Vector3, 4>& simplex) const
{
    const Vector3 a = simplex[0];
    const Vector3 b = simplex[1];
    const Vector3 c = simplex[2];
    const Vector3 d = simplex[3];

    const Vector3 n0 = (b - a).Cross(c - a);
    if (n0.Dot(a) > 0.0f && n0.Dot(d) > 0.0f)
    {
        return false;
    }

    const Vector3 n1 = (c - b).Cross(a - b);
    if (n1.Dot(b) > 0.0f && n1.Dot(a) > 0.0f)
    {
        return false;
    }

    const Vector3 n2 = (d - c).Cross(a - c);
    if (n2.Dot(c) > 0.0f && n2.Dot(b) > 0.0f)
    {
        return false;
    }

    const Vector3 n3 = (a - d).Cross(b - d);
    if (n3.Dot(d) > 0.0f && n3.Dot(c) > 0.0f)
    {
        return false;
    }

    return true;
}

PhysicsSystem::Hit PhysicsSystem::ExpandingPolytopeAlgorithm(const Body& bodyA, const Body& bodyB,
                                                             DArray<Vector3, 4>& simplex)
{
    EPAVertexArray vertices(simplex);

    DArray<EPATriangle, 16> mesh;

    std::priority_queue<EPATrianglePriority> queue;

    const Plane plane = {simplex[0], simplex[1], simplex[2]};
    if (plane.Normal().Dot(simplex[3]) < plane.D())
    {
        EPATriangle triangle(vertices, 0, 1, 2);
        mesh.Add(triangle);
        queue.push({0, triangle.Distance});

        EPATriangle triangle2(vertices, 0, 2, 3);
        mesh.Add(triangle2);
        queue.push({1, triangle2.Distance});

        EPATriangle triangle3(vertices, 0, 3, 1);
        mesh.Add(triangle3);
        queue.push({2, triangle3.Distance});

        EPATriangle triangle4(vertices, 1, 3, 2);
        mesh.Add(triangle4);
        queue.push({3, triangle4.Distance});

        mesh[0].AdjacentTriangles[0] = &mesh[1];
        mesh[0].AdjacentTriangles[1] = &mesh[2];
        mesh[0].AdjacentTriangles[2] = &mesh[3];

        mesh[1].AdjacentTriangles[0] = &mesh[0];
        mesh[1].AdjacentTriangles[1] = &mesh[2];
        mesh[1].AdjacentTriangles[2] = &mesh[3];

        mesh[2].AdjacentTriangles[0] = &mesh[0];
        mesh[2].AdjacentTriangles[1] = &mesh[1];
        mesh[2].AdjacentTriangles[2] = &mesh[3];

        mesh[3].AdjacentTriangles[0] = &mesh[0];
        mesh[3].AdjacentTriangles[1] = &mesh[1];
        mesh[3].AdjacentTriangles[2] = &mesh[2];
    }

    float distance = std::numeric_limits<float>::max();
    uint32 closestTriangleIndex = 0;
        
    std::set<EPASilhouetteEntry> silhouetteSet;

    while (!queue.empty())
    {
        EPATrianglePriority current = queue.top();
        queue.pop();
        
        EPATriangle& triangle = mesh[current.Index];
        if (!triangle.Valid)
        {
            continue;
        }

        if (current.Distance >= distance)
        {
            break;
        }

        const Vector3 v = triangle.ClosestPoint;
        Vector3 vDir = v;
        vDir.Normalize();

        float depth = current.Distance;
        Vector3 w = MinkowskiDifference(bodyA, bodyB, v);

        const float currentDepth = w.Dot(vDir);
        if (currentDepth < distance)
        {
            distance = currentDepth;
            closestTriangleIndex = current.Index;
        }

        if (depth < distance)
        {
            triangle.Valid = false;

            silhouetteSet.clear();
            for (uint8 i = 0; i < 3; ++i)
            {
                EPASilhouette(*triangle.AdjacentTriangles[i], triangle.AdjacentTriangles[i]->IndexOfAdjacend(triangle), w, silhouetteSet);
            }

            vertices.Add(w);

            for (const EPASilhouetteEntry& entry : silhouetteSet)
            {
                EPATriangle& silhouetteTriangle = *entry.Triangle;
                const uint16 indexA = silhouetteTriangle.Indices[(entry.AdjacentIndex + 1) % 3];
                const uint16 indexB = silhouetteTriangle.Indices[entry.AdjacentIndex];

                EPATriangle newTriangle(vertices, indexA, indexB, static_cast<uint16>(vertices.Count() - 1));

                EPATriangle& newTriangleRef = mesh.Add(newTriangle);

                newTriangleRef.Indices[0] = indexA;
                newTriangleRef.Indices[1] = indexB;
                newTriangleRef.Indices[2] = static_cast<uint16>(vertices.Count()) - 1;

                newTriangleRef.AdjacentTriangles[0] = &silhouetteTriangle;
                silhouetteTriangle.AdjacentTriangles[entry.AdjacentIndex] = &newTriangleRef;
            }

            const uint32 silhouetteCount = static_cast<uint32>(silhouetteSet.size());
            const uint32 startIndex = static_cast<uint32>(mesh.Count()) - silhouetteCount;
            for (uint32 i = 0; i < silhouetteCount; ++i)
            {
                EPATriangle& newTriangle = mesh[startIndex + i];
                EPATriangle& adjacentTriangle = mesh[startIndex + ((i + 1) % silhouetteCount)];

                if (&newTriangle == &adjacentTriangle)
                {
                    continue;
                }
                
                newTriangle.AdjacentTriangles[1] = &adjacentTriangle;
                adjacentTriangle.AdjacentTriangles[2] = &newTriangle;

                if (newTriangle.ContainsClosestPoint)
                {
                    queue.push({static_cast<uint16>(mesh.Count() - 1), newTriangle.Distance});
                }
            }
        }
    }

    EPATriangle& closestTriangle = mesh[closestTriangleIndex];

    Hit hit;
    hit.IsValid = true;
    hit.PenetrationDepth = distance;
    hit.OtherBody = const_cast<Body*>(&bodyB);
    hit.ImpactNormal = closestTriangle.Normal;
    hit.Location = closestTriangle.ClosestPoint;

    return hit;
}

PhysicsSystem::EPATriangle::EPATriangle(const EPAVertexArray& vertices, uint16 indexA, uint16 indexB, uint16 indexC)
{
    const Vector3 a = vertices[indexA];
    const Vector3 b = vertices[indexB];
    const Vector3 c = vertices[indexC];
    const Plane plane = {a, b, c};

    Vector3 closestPoint = Math::PlanarProjection(Vector3::Zero, a, plane.Normal());
    const bool isInside = Math::IsPointInsideTriangle(closestPoint, a, b, c);

    ClosestPoint = closestPoint;
    Distance = Math::Abs(plane.D());
    
    Normal = plane.Normal();
    if (a.Dot(Normal) > 0.0f)
    {
        Normal = -Normal;
        
        Indices[0] = indexA;
        Indices[1] = indexC;
        Indices[2] = indexB;
    }
    else
    {
        Indices[0] = indexA;
        Indices[1] = indexB;
        Indices[2] = indexC;
    }

    ContainsClosestPoint = isInside;

    AdjacentTriangles[0] = nullptr;
    AdjacentTriangles[1] = nullptr;
    AdjacentTriangles[2] = nullptr;
}

uint8 PhysicsSystem::EPATriangle::IndexOfAdjacend(const EPATriangle& other) const
{
    for (uint8 i = 0; i < 3; ++i)
    {
        if (other.AdjacentTriangles[i] == this)
        {
            return i;
        }
    }

    DEBUG_BREAK();
    return 0;
}

void PhysicsSystem::EPASilhouette(EPATriangle& triangle, uint8 adjIndex, const Vector3& w,
                                  std::set<EPASilhouetteEntry>& silhouetteSet)
{
    if (!triangle.Valid)
    {
        return;
    }

    if (triangle.ClosestPoint.Dot(w) < Math::Square(triangle.Distance))
    {
        silhouetteSet.insert({&triangle, adjIndex});
        return;
    }

    triangle.Valid = false;

    if (EPATriangle* adjTriangle = triangle.AdjacentTriangles[(adjIndex + 1) % 3])
    {
        EPASilhouette(*adjTriangle, adjTriangle->IndexOfAdjacend(triangle), w, silhouetteSet);
    }
    
    if (EPATriangle* adjTriangle = triangle.AdjacentTriangles[(adjIndex + 2) % 3])
    {
        EPASilhouette(*adjTriangle, adjTriangle->IndexOfAdjacend(triangle), w, silhouetteSet);
    }
}
