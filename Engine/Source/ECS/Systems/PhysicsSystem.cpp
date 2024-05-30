#include "PhysicsSystem.h"
#include "ECS/Components/CStaticMesh.h"
#include "Math/Math.h"
#include <queue>

PhysicsSystem::Hit PhysicsSystem::Raycast(const Vector3& start, const Vector3& direction, float distance) const
{
    const Vector3 end = start + direction * distance;
    return Raycast(start, end);
}

PhysicsSystem::Hit PhysicsSystem::Raycast(const Vector3& start, const Vector3& end) const
{
    const int32 stepX = Math::Sign(end.x - start.x);
    const int32 stepY = Math::Sign(end.y - start.y);
    const int32 stepZ = Math::Sign(end.z - start.z);

    float tMaxX;
    if (stepX > 0)
    {
        tMaxX = (1.0f + start.x / _cellSize.x) * _cellSize.x - start.x;
    }
    else
    {
        tMaxX = start.x - (start.x / _cellSize.x) * _cellSize.x;
    }

    float tMaxY;
    if (stepY > 0)
    {
        tMaxY = (1.0f + start.y / _cellSize.y) * _cellSize.y - start.y;
    }
    else
    {
        tMaxY = start.y - (start.y / _cellSize.y) * _cellSize.y;
    }

    float tMaxZ;
    if (stepZ > 0)
    {
        tMaxZ = (1.0f + start.z / _cellSize.z) * _cellSize.z - start.z;
    }
    else
    {
        tMaxZ = start.z - (start.z / _cellSize.z) * _cellSize.z;
    }

    Vector3 direction = end - start;
    direction.Normalize();
    
    Vector3 delta = direction * _cellSize;
    
    CellIndex currentIndex = GetCellIndex(start);
    auto [justOutX, justOutY, justOutZ] = GetCellIndex(end);
    justOutX += stepX;
    justOutY += stepY;
    justOutZ += stepZ;

    Line line = {start, end};
    
    while (true)
    {
        if (tMaxX < tMaxY)
        {
            if (tMaxX < tMaxZ)
            {
                currentIndex.X += stepX;
                if (currentIndex.X == justOutX)
                {
                    break;
                }
                tMaxX = tMaxX + delta.x;
            }
            else
            {
                currentIndex.Z += stepZ;
                if (currentIndex.Z == justOutZ)
                {
                    break;
                }
                tMaxZ = tMaxZ + delta.z;
            }
        }
        else
        {
            if (tMaxY < tMaxZ)
            {
                currentIndex.Y += stepY;
                if (currentIndex.Y == justOutY)
                {
                    break;
                }
                tMaxY = tMaxY + delta.y;
            }
            else
            {
                currentIndex.Z += stepZ;
                if (currentIndex.Z == justOutZ)
                {
                    break;
                }
                tMaxZ = tMaxZ + delta.z;
            }
        }
        
        const Cell& currentCell = GetCellAt(currentIndex);
        for (const Body* body : currentCell.Bodies)
        {
            if (body->AABB.Overlap(line))
            {
                const Hit hit = CollisionCheck(line, *body);
                if (hit.IsValid)
                {
                    return hit;
                }
            }
        } 
    }

    return {};
}

void PhysicsSystem::Initialize()
{
    System::Initialize();

    _cellSize = (GetWorld().WorldBounds.GetExtent() * 2.0f / _cellCountX);
}

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
    if (collider.Bounds.GetMax() - collider.Bounds.GetMin() == Vector3::Zero)
    {
        Vector3 min = collider.Bounds.GetMin();
        min.z = -0.01f;
        Vector3 max = collider.Bounds.GetMax();
        max.z = 0.01f;
        
        collider.Bounds = BoundingBox(min, max);
    }
    
    Body& body = rigidBody.PhysicsBody;
    body.Entity = &entity;
    body.AABB = collider.Bounds.TransformBy(transform.ComponentTransform);
    body.TransformIndex = transformIndex;
    body.RigidBodyIndex = rigidBodyIndex;
    body.ColliderIndex = colliderIndex;
    
    ERigidBodyState state = rigidBody.State;
    ForEachCellAt(body.AABB, [&body, state](Cell& cell, uint32 index)
    {
        const uint32 indexInCell = cell.AddBody(body, state);
        body.IndicesInCells.Add(indexInCell);
    });
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

    // Hit hit = Raycast({0.0f, 0.0f, 6.0f}, {0.0f, 0.0f, -1.0f});
    // if (hit.IsValid)
    // {
    //     LOG(L"Raycast location: {}, normal: {}", hit.Location, hit.ImpactNormal);
    // }
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
    
    const CRigidBody& rigidBody = entity.Get<const CRigidBody>(archetype);

    ForEachCellAt(rigidBody.PhysicsBody.AABB, [&rigidBody, this](Cell& cell, uint32 index)
    {
        cell.RemoveBody(*this, rigidBody.PhysicsBody.IndicesInCells[index]);
    });
}

uint32 PhysicsSystem::Cell::AddBody(Body& body, ERigidBodyState bodyType)
{
    Bodies.Resize(StaticBodyCount + DormantBodyCount + DynamicBodyCount + 1);

    uint32 bodyIndex = 0;
    
    switch (bodyType)
    {
        case ERigidBodyState::Static:
        {
            bodyIndex = StaticBodyCount;
            
            const uint32 firstDynamicIndex = StaticBodyCount + DormantBodyCount;
            const uint32 lastDynamicIndex = firstDynamicIndex + DynamicBodyCount;

            if (DynamicBodyCount > 0)
            {
                Body* firstDynamicBody = Bodies[firstDynamicIndex];
                bodyIndex = lastDynamicIndex;
                Bodies[lastDynamicIndex] = firstDynamicBody;
            }

            if (DormantBodyCount > 0)
            {
                Body* firstDormantBody = Bodies[StaticBodyCount];
                bodyIndex = firstDynamicIndex;
                
                Bodies[firstDynamicIndex] = firstDormantBody;
                Bodies[StaticBodyCount] = &body;
            }
            else
            {
                Bodies[StaticBodyCount] = &body;
            }
            
            ++StaticBodyCount;
            
            break;
        }

        case ERigidBodyState::Dormant:
        {
            const uint32 firstDynamicIndex = StaticBodyCount + DormantBodyCount;
            const uint32 lastDynamicIndex = firstDynamicIndex + DynamicBodyCount;

            Body* firstDynamicBody = Bodies[firstDynamicIndex];
            if (DynamicBodyCount > 0)
            {
                bodyIndex = lastDynamicIndex;
                Bodies[lastDynamicIndex] = firstDynamicBody;
            }
            else
            {
                bodyIndex = firstDynamicIndex;
                Bodies[firstDynamicIndex] = &body;
            }

            ++DormantBodyCount;
            break;
        }
        
        case ERigidBodyState::Dynamic:
        {
            const uint32 index = StaticBodyCount + DormantBodyCount + DynamicBodyCount;
            bodyIndex = index;
            
            Bodies[index] = &body;
            
            ++DynamicBodyCount;
            break;
        }
    }

    return bodyIndex;
}

void PhysicsSystem::Cell::RemoveBody(const PhysicsSystem& system, uint32 index)
{
    if (index >= StaticBodyCount + DormantBodyCount)
    {
        if (StaticBodyCount + DormantBodyCount > 0)
        {
            const uint32 lastDynamicIndex = StaticBodyCount + DormantBodyCount + DynamicBodyCount - 1;
            if (index < lastDynamicIndex)
            {
                Body* lastDynamicBody = Bodies[lastDynamicIndex];
                lastDynamicBody->IndicesInCells[system.GetRelativeIndexOf(*lastDynamicBody, *this)] = index;
                
                Bodies[index] = lastDynamicBody;
            }
        }

        --DynamicBodyCount;
    }
    else if (index >= StaticBodyCount)
    {
        const uint32 lastDormantIndex = StaticBodyCount + DormantBodyCount - 1;
        Body* lastDormantBody = Bodies[lastDormantIndex];
        lastDormantBody->IndicesInCells[system.GetRelativeIndexOf(*lastDormantBody, *this)] = index;
        Bodies[index] = lastDormantBody;

        Body* lastDynamicBody = Bodies[StaticBodyCount + DormantBodyCount + DynamicBodyCount - 1];
        lastDynamicBody->IndicesInCells[system.GetRelativeIndexOf(*lastDynamicBody, *this)] = lastDormantIndex;
        Bodies[lastDormantIndex] = lastDynamicBody;

        --DormantBodyCount;
    }
    else
    {
        const uint32 lastStaticIndex = StaticBodyCount - 1;

        Body** lastStaticBody = &Bodies[lastStaticIndex];
        (*lastStaticBody)->IndicesInCells[system.GetRelativeIndexOf(**lastStaticBody, *this)] = index;
        Bodies[index] = *lastStaticBody;

        Body** lastDormantBody = &Bodies[lastStaticIndex + DormantBodyCount];
        (*lastDormantBody)->IndicesInCells[system.GetRelativeIndexOf(**lastDormantBody, *this)] = lastStaticIndex;
        *lastStaticBody = *lastDormantBody;

        Body** lastDynamicBody = &Bodies[StaticBodyCount + DormantBodyCount + DynamicBodyCount - 1];
        (*lastDormantBody)->IndicesInCells[system.GetRelativeIndexOf(**lastDormantBody, *this)] = lastStaticIndex + DormantBodyCount;
        *lastDormantBody = *lastDynamicBody;

        StaticBodyCount--;
    }

    Bodies.RemoveAt(StaticBodyCount + DormantBodyCount + DynamicBodyCount);
}

void PhysicsSystem::Cell::SetBodyState(uint32 index, ERigidBodyState bodyType)
{
}

PhysicsSystem::CellIndex PhysicsSystem::GetCellIndex(const Vector3& location) const
{
    const BoundingBox& worldBounds = GetWorld().WorldBounds;
    const Vector3 relativeLocation = location - worldBounds.GetMin();

    const Vector3 index3D = relativeLocation / _cellSize;
    
    CellIndex index;
    index.X = static_cast<uint32>(Math::FloorToInt(index3D.x));
    index.Y = static_cast<uint32>(Math::FloorToInt(index3D.y));
    index.Z = static_cast<uint32>(Math::FloorToInt(index3D.z));

    return index;
}

PhysicsSystem::Cell& PhysicsSystem::GetCellAt(const Vector3& location)
{
    return const_cast<Cell&>(GetCellAt(GetCellIndex(location)));
}

const PhysicsSystem::Cell& PhysicsSystem::GetCellAt(const Vector3& location) const
{
    return GetCellAt(GetCellIndex(location));
}

PhysicsSystem::Cell& PhysicsSystem::GetCellAt(const CellIndex& index)
{
    return const_cast<Cell&>(GetCellAtImplementation(index));
}

const PhysicsSystem::Cell& PhysicsSystem::GetCellAt(const CellIndex& index) const
{
    return GetCellAtImplementation(index);
}

uint32 PhysicsSystem::GetRelativeIndexOf(const Body& body, const Cell& cell) const
{
    const CellIndex minIndex = GetCellIndex(body.AABB.GetMin());

    return cell.Index.Z - minIndex.Z +
        (cell.Index.Y - minIndex.Y) * _cellCountX +
        (cell.Index.X - minIndex.X) * _cellCountX * _cellCountX;
}

void PhysicsSystem::ForEachCellAt(const BoundingBox& aabb, const std::function<void(Cell& cell, uint32 index)>& func)
{
    const CellIndex minIndex = GetCellIndex(aabb.GetMin());
    const CellIndex maxIndex = GetCellIndex(aabb.GetMax());

    uint32 i = 0;
    for (uint32 x = minIndex.X; x <= maxIndex.X; ++x)
    {
        for (uint32 y = minIndex.Y; y <= maxIndex.Y; ++y)
        {
            for (uint32 z = minIndex.Z; z <= maxIndex.Z; ++z)
            {
                CellIndex index = {x, y, z};
                func(GetCellAt(index), i);

                ++i;
            }
        }
    }
}

const PhysicsSystem::Cell& PhysicsSystem::GetCellAtImplementation(const CellIndex& index) const
{
    const uint32 index1D = index.X + index.Y * _cellCountX + index.Z * _cellCountX * _cellCountX;

    auto it = _cells.find(index1D);
    if (it != _cells.end())
    {
        return it->second;
    }

    Cell& newCell = const_cast<std::unordered_map<uint32, Cell>&>(_cells)[index1D];
    newCell.Index = index;  
    
    return newCell;
}

void PhysicsSystem::Move(CRigidBody& rigidBody, const Vector3& currentLocation, const Vector3& newLocation, double deltaTime)
{
    Body& body = rigidBody.PhysicsBody;
    const BoundingBox currentAABB = body.AABB;

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
        for (Body* cellBody : cellAtSweep.Bodies)
        {
            if (cellBody->Entity == body.Entity)
            {
                continue;
            }
        
            if (sweepAABB.Overlap(cellBody->AABB))
            {
                const Vector3 bodyLocation = body.AABB.GetCenter() + offset;
                Transform& transform = body.GetTransform().ComponentTransform;
                transform.SetWorldLocation(bodyLocation);
                
                Hit hit = CollisionCheck(body, *cellBody);
                
                if (hit.IsValid)
                {
                    ProcessHit(body, hit);
                    // LOG(L"Hit location {}, normal {}", hit.Location, hit.ImpactNormal);
                    
                    const Vector3 location = body.AABB.GetCenter();
                    body.AABB.Move(hit.ImpactNormal * hit.PenetrationDepth);
                    if (hit.PenetrationDepth > 1.0f)
                    {
                        __nop();
                    }
                    LOG(L"Resolving penetration: normal {}, depth {}, location before {}, location after {}",
                        hit.ImpactNormal,
                        hit.PenetrationDepth,
                        location,
                        body.AABB.GetCenter()
                    );

                    _narrowPhaseInputPairs.Add({&body, hit.OtherBody});

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

    Transform& transform = body.GetTransform().ComponentTransform;
    transform.SetWorldLocation(location);

    const Vector3 deltaRotationRad = rigidBody.AngularVelocity * static_cast<float>(deltaTime);
    transform.SetWorldRotation(
        transform.GetWorldRotationEuler() + Math::ToDegrees(deltaRotationRad)
    );

    Move(rigidBody, currentAABB, body.AABB);
}

void PhysicsSystem::Move(CRigidBody& rigidBody, const BoundingBox& current, const BoundingBox& next)
{
    Body& body = rigidBody.PhysicsBody;

    const CellIndex currentMin = GetCellIndex(current.GetMin());
    const CellIndex currentMax = GetCellIndex(current.GetMax());
    const CellIndex newMin = GetCellIndex(next.GetMin());
    const CellIndex newMax = GetCellIndex(next.GetMax());


    CellIndex intersectMin;
    intersectMin.X = Math::Max(currentMin.X, newMin.X);
    intersectMin.Y = Math::Max(currentMin.Y, newMin.Y);
    intersectMin.Z = Math::Max(currentMin.Z, newMin.Z);
    
    CellIndex intersectMax;
    intersectMax.X = Math::Min(currentMax.X, newMax.X);
    intersectMax.Y = Math::Min(currentMax.Y, newMax.Y);
    intersectMax.Z = Math::Min(currentMax.Z, newMax.Z);

    uint32 index;
    for (uint32 x = currentMin.X; x < intersectMin.X; ++x)
    {
        for (uint32 y = currentMin.Y; y < intersectMin.Y; ++y)
        {
            index = x * _cellCountX * _cellCountX + currentMin.Y * _cellCountX + currentMin.Z;
            
            for (uint32 z = currentMin.Z; z < intersectMin.Z; ++z)
            {
                Cell& cell = GetCellAt(CellIndex(x, y, z));
                cell.RemoveBody(*this, index);

                ++index;
            }
        }
    }
    
    for (uint32 x = intersectMax.X + 1; x <= currentMax.X; ++x)
    {
        for (uint32 y = intersectMax.Y + 1; y <= currentMax.Y; ++y)
        {
            index = x * _cellCountX * _cellCountX + currentMin.Y * _cellCountX + intersectMax.Z + 1;
            
            for (uint32 z = intersectMax.Z + 1; z <= currentMax.Z; ++z)
            {
                Cell& cell = GetCellAt(CellIndex(x, y, z));
                cell.AddBody(body, rigidBody.State);

                ++index;
            }
        }
    }
}

void PhysicsSystem::BroadPhase(Body& body, Cell& cell)
{
    for (Body* cellBody : cell.Bodies)
    {
        if (cellBody->Entity == body.Entity)
        {
            continue;
        }
        
        if (body.AABB.Overlap(cellBody->AABB))
        {
            _narrowPhaseInputPairs.Add({&body, cellBody});
        }
    }
}

void PhysicsSystem::NarrowPhase()
{
    // for (const CollisionPair& pair : _narrowPhaseInputPairs)
    // {
    //     CRigidBody& rigidBodyA = pair.BodyA->GetRigidBody();
    //     CRigidBody& rigidBodyB = pair.BodyB->GetRigidBody();
    //
    //     const Vector3 velocityA = rigidBodyA.Velocity;
    //     const Vector3 velocityB = rigidBodyB.Velocity;
    //
    //     Hit hit = CollisionCheck(*pair.BodyA, *pair.BodyB);
    //
    //     // todo update overlaps
    // }
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

    //LOG(L"Velocity after hit: {}", rigidBodyA.Velocity);
}

Vector3 PhysicsSystem::Support(const Body& body, const Vector3& directionNormalized) const
{
    Vector3 furthestPointA;
    Vector3 transformedDirectionA = Vector3::TransformNormal(directionNormalized, body.GetTransform().ComponentTransform.GetWorldMatrix().Invert());
    transformedDirectionA.Normalize();
    std::visit([&transformedDirectionA, &furthestPointA](auto&& arg)
    {
        furthestPointA = arg.FurthestPointInDirection(transformedDirectionA);
    },
    body.GetCollider().Shape);

    return body.GetTransform().ComponentTransform * furthestPointA;
}

Vector3 PhysicsSystem::Support(const Line& line, const Vector3& directionNormalized) const
{
    Vector3 rayDirection = line.End - line.Start;
    rayDirection.Normalize();

    const float projection = directionNormalized.Dot(rayDirection);
    if (projection > 0.0f)
    {
        return line.End;
    }
    
    return line.Start;
}

Vector3 PhysicsSystem::MinkowskiDifference(const Body& bodyA, const Body& bodyB, const Vector3& direction) const
{
    Vector3 directionNormalized = direction;
    directionNormalized.Normalize();
    
    const Vector3 furthestPointA = Support(bodyA, directionNormalized);
    const Vector3 furthestPointB = Support(bodyB, -directionNormalized);

    return furthestPointA - furthestPointB;
}

Vector3 PhysicsSystem::MinkowskiDifference(const Line& line, const Body& bodyB, const Vector3& direction) const
{
    Vector3 directionNormalized = direction;
    directionNormalized.Normalize();
    
    const Vector3 furthestPointA = Support(line, directionNormalized);
    
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

bool PhysicsSystem::GJKSimplexContainsOrigin(DArray<GJKVertex, 4>& simplex, Vector3& direction) const
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
            const GJKVertex a = simplex[0];
            const GJKVertex b = simplex[1];
            const GJKVertex c = simplex[2];
            const GJKVertex d = simplex[3];

            const Vector3 ab = b.Location - a.Location;
            const Vector3 ac = c.Location - a.Location;

            const Vector3 aOrigin = -a.Location;
            
            const Vector3 abc = ab.Cross(ac);
            if (abc.Dot(aOrigin) > 0.0f)
            {
                simplex = {a, b, c};
                return GJKTriangle(simplex, direction);
            }

            const Vector3 ad = d.Location - a.Location;
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

bool PhysicsSystem::GJKLine(DArray<GJKVertex, 4>& simplex, Vector3& direction) const
{
    const GJKVertex a = simplex[0];
    const GJKVertex b = simplex[1];

    const Vector3 ab = b.Location - a.Location;
    const Vector3 aOrigin = -a.Location;

    if (ab.Dot(aOrigin) > 0.0f)
    {
        direction = ab.Cross(aOrigin).Cross(ab);
    }
    else
    {
        direction = aOrigin;
        simplex = {a};
    }

    direction.Normalize();
    
    return false;
}

bool PhysicsSystem::GJKTriangle(DArray<GJKVertex, 4>& simplex, Vector3& direction) const
{
    const GJKVertex a = simplex[0];
    const GJKVertex b = simplex[1];
    const GJKVertex c = simplex[2];

    const Vector3 ab = b.Location - a.Location;
    const Vector3 ac = c.Location - a.Location;
    const Vector3 up = ab.Cross(ac);

    const Vector3 aOrigin = -a.Location;

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

    direction.Normalize();
    
    return false;
}

bool PhysicsSystem::TetrahedronContainsOrigin(const EPAVertexArray& polytopeVertices) const
{
    const Vector3 a = polytopeVertices[0].Location;
    const Vector3 b = polytopeVertices[1].Location;
    const Vector3 c = polytopeVertices[2].Location;
    const Vector3 d = polytopeVertices[3].Location;

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

template <typename ShapeProxyType>
PhysicsSystem::Hit PhysicsSystem::ExpandingPolytopeAlgorithm(const ShapeProxyType& shape, const Body& bodyB, DArray<GJKVertex, 4>& simplex) const
{
    EPAVertexArray vertices(simplex);
    if (TetrahedronContainsOrigin(vertices))
    {
        return {};
    }

    DArray<EPATriangle, 16> mesh;

    std::priority_queue<EPATrianglePriority> queue;
    
    EPATriangle triangle1(vertices, 0, 1, 2);
    mesh.Add(triangle1);
    queue.push({0, triangle1.Distance});

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

    float distance = std::numeric_limits<float>::max();
    uint32 closestTriangleIndex = 0;
        
    EPASilhouetteArray silhouetteArray;

    while (!queue.empty())
    {
        EPATrianglePriority current = queue.top();
        queue.pop();

        if (current.Distance >= distance)
        {
            break;
        }
        
        EPATriangle& triangle = mesh[current.Index];
        if (!triangle.Valid)
        {
            continue;
        }
        
        const Vector3 v = triangle.ClosestPoint;
        
        const Vector3 furthestPointA = Support(shape, v);
        const Vector3 furthestPointB = Support(bodyB, -v);

        const Vector3 w = furthestPointA - furthestPointB;

        Vector3 vDir = v;
        vDir.Normalize();
        
        const float currentDepth = w.Dot(vDir);
        if (currentDepth < distance)
        {
            distance = currentDepth;
            closestTriangleIndex = current.Index;
        }

        if (current.Distance < distance)
        {
            triangle.Valid = false;

            silhouetteArray.Clear();
            for (uint8 i = 0; i < 3; ++i)
            {
                if (triangle.AdjacentTriangles[i] == nullptr)
                {
                    continue;
                }
                
                EPASilhouette(*triangle.AdjacentTriangles[i], triangle.AdjacentTriangles[i]->IndexOfAdjacent(triangle), w, silhouetteArray);
            }

            if (silhouetteArray.Count() < 3)
            {
                continue;
            }

            vertices.Add({w, furthestPointA, furthestPointB});

            for (const EPASilhouetteEntry& entry : silhouetteArray)
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

            const uint32 silhouetteCount = static_cast<uint32>(silhouetteArray.Count());
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
    hit.PenetrationDepth = Math::Abs(closestTriangle.Normal.Dot(vertices[closestTriangle.Indices[0]].Location));
    hit.OtherBody = const_cast<Body*>(&bodyB);
    hit.ImpactNormal = -closestTriangle.Normal;
    
    const Vector3& cA = vertices[closestTriangle.Indices[0]].Location;
    const Vector3& cB = vertices[closestTriangle.Indices[1]].Location;
    const Vector3& cC = vertices[closestTriangle.Indices[2]].Location;

    const Vector3 cP = closestTriangle.ClosestPoint;
    Vector3 baryCoords = Math::BarycentricCoordinates(cP, cA, cB, cC);

    const GJKVertex& vA = vertices[closestTriangle.Indices[0]];
    const GJKVertex& vB = vertices[closestTriangle.Indices[1]];
    const GJKVertex& vC = vertices[closestTriangle.Indices[2]];

    const Vector3 contactPointA = baryCoords.x * vA.SupportA + baryCoords.y * vB.SupportA + baryCoords.z * vC.SupportA;
    // todo once we implement hit events, we will need to broadcast both hits
    // const Vector3 contactPointB = baryCoords.x * vA.SupportB + baryCoords.y * vB.SupportB + baryCoords.z * vC.SupportB;

    // LOG(L"Closest point: {}", closestTriangle.ClosestPoint);
    // LOG(L"Contact point A: {}", contactPointA);
    // LOG(L"Contact point B: {}", contactPointB);

    if (hit.ImpactNormal == Vector3::Zero)
    {
        hit.ImpactNormal = contactPointA;
        hit.ImpactNormal.Normalize();
    }
    
    hit.Location = contactPointA + hit.ImpactNormal * hit.PenetrationDepth;
    
    if (hit.Location.Length() > 10.0f)
    {
        __nop();
    }

    return hit;
}

template <typename ShapeProxyType>
PhysicsSystem::Hit PhysicsSystem::GilbertJohnsonKeerthi(const ShapeProxyType& shape, const Body& bodyB) const
{
    const Vector3 initialDirection = Vector3::UnitX;
    Vector3 furthestPointA = Support(shape, initialDirection);
    Vector3 furthestPointB = Support(bodyB, -initialDirection);

    Vector3 difference = furthestPointA - furthestPointB;

    DArray<GJKVertex, 4> simplex = {{difference, furthestPointA, furthestPointB}};

    Vector3 direction = -difference;
    direction.Normalize();
    while (true)
    {
        furthestPointA = Support(shape, direction);
        furthestPointB = Support(bodyB, -direction);

        difference = furthestPointA - furthestPointB;

        difference = MinkowskiDifference(shape, bodyB, direction);
        if (difference.Dot(direction) <= 0.0f)
        {
            // LOG(L"Hit not found");
            return {};
        }

        simplex.InsertAt(0, {difference, furthestPointA, furthestPointB});

        if (GJKSimplexContainsOrigin(simplex, direction))
        {
            // LOG(L"Hit found");
            if (simplex.Count() == 4)
            {
                return ExpandingPolytopeAlgorithm(shape, bodyB, simplex);
            }

            // todo support for dot, line and triangle simplex expansion to tetrahedron
            DEBUG_BREAK();
            return {};
        }
    }
}

template <typename ShapeProxyType>
PhysicsSystem::Hit PhysicsSystem::CollisionCheck(const ShapeProxyType& shape, const Body& bodyB) const
{
    return GilbertJohnsonKeerthi(shape, bodyB);
}

PhysicsSystem::EPATriangle::EPATriangle(const EPAVertexArray& vertices, uint16 indexA, uint16 indexB, uint16 indexC)
{
    const Vector3 a = vertices[indexA].Location;
    const Vector3 b = vertices[indexB].Location;
    const Vector3 c = vertices[indexC].Location;

    Normal = (b - a).Cross(c - a);
    Normal.Normalize();

    Vector3 closestPoint = Math::PlanarProjection(Vector3::Zero, a, Normal);
    const bool isInside = Math::IsPointInsideTriangle(closestPoint, a, b, c);

    ClosestPoint = closestPoint;
    Distance = ClosestPoint.Length();

    float dot = a.Dot(Normal);
    if (a.Dot(Normal) < 0.0f)
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

uint8 PhysicsSystem::EPATriangle::IndexOfAdjacent(const EPATriangle& other) const
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

void PhysicsSystem::EPASilhouette(EPATriangle& triangle, uint8 adjIndex, const Vector3& w, EPASilhouetteArray& silhouetteSet) const
{
    if (!triangle.Valid)
    {
        return;
    }

    if (triangle.ClosestPoint.Dot(w) < Math::Square(triangle.Distance))
    {
        silhouetteSet.Add({&triangle, adjIndex});
        return;
    }

    triangle.Valid = false;

    if (EPATriangle* adjTriangle = triangle.AdjacentTriangles[(adjIndex + 1) % 3])
    {
        EPASilhouette(*adjTriangle, adjTriangle->IndexOfAdjacent(triangle), w, silhouetteSet);
    }
    
    if (EPATriangle* adjTriangle = triangle.AdjacentTriangles[(adjIndex + 2) % 3])
    {
        EPASilhouette(*adjTriangle, adjTriangle->IndexOfAdjacent(triangle), w, silhouetteSet);
    }
}
