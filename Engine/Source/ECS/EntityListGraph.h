#pragma once

#include "EntityList.h"
#include "Archetype.h"
#include <vector>

class EntityListGraph
{
public:
    struct Node : public IValidateable
    {
    public:
        friend class IValidateable;
    public:
        // todo SSO vector must be used here
        std::vector<Node*> Parents;
        std::vector<Node*> Children;

        EntityList EntityList;

    public:
        explicit Node();
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
    
public:
    explicit EntityListGraph();

    Node* AddArchetype(const Archetype& type);
    void RemoveArchetype(const Archetype& type);

    bool Traverse(Node* root, const std::function<bool(Node*)>& callback);

    EntityList& GetOrCreateEntityListFor(const Archetype& type);
    // todo queries

private:
    BucketArray<Node> _nodes;

    Node* _root = nullptr;
    std::unordered_map<uint64, Node*> _archetypeToNodeMap;

private:
    Node* FindBestMatch(const Archetype& type);
};
