#pragma once

#include "ECSQuery.h"
#include "EntityList.h"

class EntityListGraph
{
public:
    struct Node : public IValidateable
    {
    public:
        friend class IValidateable;

    public:
        DArray<Node*, 8> Parents;
        DArray<Node*, 8> Children;

        EntityList EntityList;

    public:
        explicit Node() = default;
        explicit Node(const Archetype& type);

        const Archetype& GetArchetype() const;

        void AddChild(Node* child, PassKey<EntityListGraph>);
        void AddChildByArchetype(Node* newChild, PassKey<EntityListGraph>);

        void RemoveChild(Node* child, PassKey<EntityListGraph>);

    private:
        bool _isValid = false;

    private:
        void SetValidImplementation(bool value);
        bool IsValidImplementation() const;
    };

    struct EntityListResult
    {
        EntityList* List = nullptr;
        bool WasCreated = false;
    };

public:
    explicit EntityListGraph();

    Node* AddArchetype(const Archetype& type);
    void RemoveArchetype(const Archetype& type);

    void Query(ECSQuery& query, const Archetype& archetype) const;

    bool Traverse(Node* root, const std::function<bool(Node*)>& callback) const;

    EntityListResult GetOrCreateEntityListFor(const Archetype& type);

private:
    BucketArray<Node> _nodes;

    Node* _root = nullptr;
    std::unordered_map<uint64, Node*> _archetypeToNodeMap;

private:
    Node* FindBestMatch(const Archetype& type) const;
};
