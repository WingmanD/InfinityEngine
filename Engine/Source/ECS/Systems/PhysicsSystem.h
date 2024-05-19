#pragma once

#include "ECS/Components/CCollider.h"
#include "ECS/Components/CRigidBody.h"
#include "ECS/Components/CTransform.h"
#include "ECS/Systems/System.h"
#include "ECS/Systems/PhysicsSystem.reflection.h"

REFLECTED()
class PhysicsSystem : public System<CTransform, CRigidBody, CCollider>
{
    GENERATED()

public:
    struct Hit
    {
        bool IsValid = false;
        Entity* Entity = nullptr;
        Vector3 Normal = Vector3::Zero;
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
    const Vector3 _gravity = Vector3(0.0f, 0.0f, -0.981f);

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
    void NarrowPhase(double deltaTime);

    Hit CollisionCheck(const Body& bodyA, const Body& bodyB);
};
