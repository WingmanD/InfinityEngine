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
    struct Hit
    {
        bool IsValid = false;
        float PenetrationDepth = 0.0f;
        Body* OtherBody = nullptr;
        Vector3 ImpactNormal = Vector3::Zero;
        Vector3 Location = Vector3::Zero;
    };

public:
    Hit Raycast(const Vector3& start, const Vector3& direction, float distance) const;
    Hit Raycast(const Vector3& start, const Vector3& end) const;
    
    // System
public:
    virtual void Initialize() override;
    virtual void OnEntityCreated(const Archetype& archetype, Entity& entity) override;
    virtual void Tick(double deltaTime) override;
    virtual void ProcessEntityList(EntityList& entityList, double deltaTime) override;
    virtual void OnEntityDestroyed(const Archetype& archetype, Entity& entity) override;

private:
    // todo entities that generate force fields
    const Vector3 _gravity = Vector3(0.0f, 0.0f, -9.81f);
    
    static constexpr uint32 _cellCountX = 100;
    static constexpr uint32 _cellSSOCapacity = 256;

    Vector3 _cellSize;
    
    struct CellIndex
    {
        uint32 X;
        uint32 Y;
        uint32 Z;
    };

    struct Cell
    {
    public:
        CellIndex Index;
        DArray<Body*, _cellSSOCapacity> Bodies;
        uint32 StaticBodyCount = 0;
        uint32 DormantBodyCount = 0;
        uint32 DynamicBodyCount = 0;

    public:
        uint32 AddBody(Body& body, ERigidBodyState bodyType);
        void RemoveBody(const PhysicsSystem& system, uint32 index);
        
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
    struct GJKVertex
    {
        Vector3 Location;
        Vector3 SupportA;
        Vector3 SupportB;
    };
    
    using EPAVertexArray = DArray<GJKVertex, 16>;
    
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

        uint8 IndexOfAdjacent(const EPATriangle& other) const;
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
    
    using EPASilhouetteArray = DArray<EPASilhouetteEntry, 8>;
    
private:
    CellIndex GetCellIndex(const Vector3& location) const;
    Cell& GetCellAt(const Vector3& location);
    const Cell& GetCellAt(const Vector3& location) const;
    Cell& GetCellAt(const CellIndex& index);
    const Cell& GetCellAt(const CellIndex& index) const;

    uint32 GetRelativeIndexOf(const Body& body, const Cell& cell) const;

    void ForEachCellAt(const BoundingBox& aabb, const std::function<void(Cell& cell, uint32 index)>& func);

    const Cell& GetCellAtImplementation(const CellIndex& index) const;
    
    void Move(CRigidBody& rigidBody, const Vector3& currentLocation, const Vector3& newLocation, double deltaTime);
    void Move(CRigidBody& rigidBody, const BoundingBox& current, const BoundingBox& next);

    void BroadPhase(Body& body, Cell& cell);
    void NarrowPhase();
    
    void ProcessHit(const Body& bodyA, const Hit& hit);

    Vector3 Support(const Body& body, const Vector3& directionNormalized) const;
    Vector3 Support(const Line& line, const Vector3& directionNormalized) const;
    
    Vector3 MinkowskiDifference(const Body& bodyA, const Body& bodyB, const Vector3& direction) const;
    Vector3 MinkowskiDifference(const Line& line, const Body& bodyB, const Vector3& direction) const;
    bool GJKSimplexContainsOrigin(DArray<GJKVertex, 4>& simplex, Vector3& direction) const;
    bool GJKLine(DArray<GJKVertex, 4>& simplex, Vector3& direction) const;
    bool GJKTriangle(DArray<GJKVertex, 4>& simplex, Vector3& direction) const;

    bool TetrahedronContainsOrigin(const EPAVertexArray& polytopeVertices) const;

    template <typename ShapeProxyType>
    Hit ExpandingPolytopeAlgorithm(const ShapeProxyType& shape, const Body& bodyB, DArray<GJKVertex, 4>& simplex) const;

    template <typename ShapeProxyType>
    Hit GilbertJohnsonKeerthi(const ShapeProxyType& shape, const Body& bodyB) const;

    template <typename ShapeProxyType>
    Hit CollisionCheck(const ShapeProxyType& shape, const Body& bodyB) const;

    void EPASilhouette(EPATriangle& triangle, uint8 adjIndex, const Vector3& w, EPASilhouetteArray& silhouetteSet) const;
};
