#pragma once

#include "ECS/Components/CCollider.h"
#include "ECS/Components/CRigidBody.h"
#include "ECS/Components/CTransform.h"
#include "ECS/Systems/System.h"
#include "Math/Math.h"
#include "ECS/Systems/PhysicsSystem.reflection.h"

REFLECTED()
class PhysicsSystem : public System<CTransform, CRigidBody, CCollider>
{
    GENERATED()

public:
    struct Body
    {
        Entity* Entity;
        BoundingBox AABB;
        uint16 TransformIndex;
        uint16 RigidBodyIndex;
        uint16 ColliderIndex;

        CTransform& GetTransform() const;
        CRigidBody& GetRigidBody() const;
        CCollider& GetCollider() const;
    };
    
    struct Hit
    {
        bool IsValid = false;
        float PenetrationDepth = 0.0f;
        Body* OtherBody = nullptr;
        Vector3 ImpactNormal = Vector3::Zero;
        Vector3 Location = Vector3::Zero;
    };
    
    // System
public:
    virtual void OnEntityCreated(const Archetype& archetype, Entity& entity) override;
    virtual void Tick(double deltaTime) override;
    virtual void ProcessEntityList(EntityList& entityList, double deltaTime) override;
    virtual void OnEntityDestroyed(const Archetype& archetype, Entity& entity) override;

private:
    // todo entities that generate force fields
    const Vector3 _gravity = Vector3(0.0f, 0.0f, -9.81f);
    
    static constexpr uint32 _cellCountX = 100;
    static constexpr uint32 _cellSSOCapacity = 256;

    struct Cell
    {
    public:
        DArray<Body, _cellSSOCapacity> Bodies;
        uint32 StaticBodyCount = 0;
        uint32 DormantBodyCount = 0;
        uint32 DynamicBodyCount = 0;

    public:
        void AddBody(const Body& body, ERigidBodyState bodyType);
        void RemoveBody(uint32 index);
        
        void SetBodyState(uint32 index, ERigidBodyState bodyType);
    };
    
    std::unordered_map<uint32, Cell> _cells;

    // BodyA is the entity with the lower ID
    struct CollisionPair
    {
        Body* BodyA;
        Body* BodyB;
    };

    std::unordered_map<Entity*, CollisionPair> _overlaps;
    
    DArray<CollisionPair> _narrowPhaseInputPairs;

private:
    Cell& GetCellAt(const Vector3& location);
    void Move(const CRigidBody& rigidBody, const Vector3& currentLocation, const Vector3& newLocation, double deltaTime);
    void Move(const CRigidBody& rigidBody, Cell& currentCell, Cell& newCell);

    void BroadPhase(Body& body, Cell& cell);
    void NarrowPhase();

    Hit CollisionCheck(const Body& bodyA, const Body& bodyB, const Vector3& movementDirection);
    Hit CollisionCheck(const Body& bodyA, const Body& bodyB);
    void ProcessHit(const Body& bodyA, const Hit& hit);
    
    Hit GilbertJohnsonKeerthi(const Body& bodyA, const Body& bodyB);
    Vector3 MinkowskiDifference(const Body& bodyA, const Body& bodyB, const Vector3& direction);
    bool GJKSimplexContainsOrigin(DArray<Vector3, 4>& simplex, Vector3& direction) const;
    bool GJKLine(DArray<Vector3, 4>& simplex, Vector3& direction) const;
    bool GJKTriangle(DArray<Vector3, 4>& simplex, Vector3& direction) const;

    bool TetrahedronContainsOrigin(const DArray<Vector3, 4>& simplex) const;

    Hit ExpandingPolytopeAlgorithm(const Body& bodyA, const Body& bodyB, DArray<Vector3, 4>& simplex);

    using EPAVertexArray = DArray<Vector3, 16>;
    
    struct EPATriangle
    {
    public:
        uint16 Indices[3];
        Vector3 ClosestPoint;
        float Distance;
        Vector3 Normal;
        bool ContainsClosestPoint;
        bool Valid = true;

        EPATriangle* AdjacentTriangles[3];

    public:
        EPATriangle(const EPAVertexArray& vertices, uint16 indexA, uint16 indexB, uint16 indexC);

        uint8 IndexOfAdjacend(const EPATriangle& other) const;
    };
    
    struct EPATrianglePriority
    {
    public:
        uint16 Index;
        float Distance;

    public:
        auto operator<=>(const EPATrianglePriority& other) const
        {
            return Distance <=> other.Distance;
        }
    };

    struct EPASilhouetteEntry
    {
    public:
        EPATriangle* Triangle;
        uint8 AdjacentIndex;

    public:
        auto operator<=>(const EPASilhouetteEntry& other) const
        {
            return std::tie(Triangle, AdjacentIndex) <=> std::tie(other.Triangle, other.AdjacentIndex);
        }
    };

    void EPASilhouette(EPATriangle& triangle, uint8 adjIndex, const Vector3& w, std::set<EPASilhouetteEntry>& silhouetteSet);
};
