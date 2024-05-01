﻿#include "EntityListGraph.h"
#include "Archetype.h"

EntityListGraph::Node::Node() : EntityList(Archetype())
{
}

EntityListGraph::Node::Node(const Archetype& type): EntityList(type)
{
}

const Archetype& EntityListGraph::Node::GetArchetype() const
{
    return EntityList.GetArchetype();
}

void EntityListGraph::Node::AddChild(Node* child, PassKey<EntityListGraph>)
{
    Children.Add(child);
    child->Parents.Add(this);
}

void EntityListGraph::Node::AddChildByArchetype(Node* newChild, PassKey<EntityListGraph>)
{
    const Archetype& type = newChild->GetArchetype();
    for (Node* child : Children)
    {
        if (child->GetArchetype().IsSupersetOf(type))
        {
            newChild->AddChild(child, {});
        }
    }
    for (Node* child : newChild->Children)
    {
        RemoveChild(child, {});
    }

    AddChild(newChild, {});
}

void EntityListGraph::Node::RemoveChild(Node* child, PassKey<EntityListGraph>)
{
    child->Parents.Remove(this);
    Children.Remove(child);
}

void EntityListGraph::Node::SetValidImplementation(bool value)
{
    _isValid = value;
}

bool EntityListGraph::Node::IsValidImplementation() const
{
    return _isValid;
}

EntityListGraph::EntityListGraph()
{
    _root = _nodes.AddDefault();
}

EntityListGraph::Node* EntityListGraph::AddArchetype(const Archetype& type)
{
    assert(!_archetypeToNodeMap.contains(type.GetID()));

    Node* node = _nodes.Emplace(type);

    Node* bestMatchNode = FindBestMatch(type);
    if (bestMatchNode == nullptr)
    {
        _root->AddChildByArchetype(node, {});
        _archetypeToNodeMap[type.GetID()] = node;
        return node;
    }

    if (bestMatchNode->GetArchetype().IsSupersetOf(type))
    {
        _archetypeToNodeMap[type.GetID()] = node;

        for (int64 i = bestMatchNode->Parents.Count() - 1; i >= 0; --i)
        {
            Node* parent = bestMatchNode->Parents[i];
            
            if (parent->GetArchetype().IsSubsetOf(type))
            {
                parent->AddChildByArchetype(node, {});
                
                bestMatchNode->Parents.RemoveAtSwap(i);
            }
        }

        node->AddChildByArchetype(bestMatchNode, {});
        
        return node;
    }

    bestMatchNode->AddChildByArchetype(node, {});

    if (type == bestMatchNode->GetArchetype())
    {
        return node;
    }

    Archetype matched = bestMatchNode->GetArchetype();

    while (matched != type)
    {
        const Archetype difference = type.Difference(matched);
        if (difference.GetID() == 0)
        {
            break;
        }

        Node* bestDifferenceMatch = FindBestMatch(difference);
        if (bestDifferenceMatch == nullptr)
        {
            bestDifferenceMatch = _root;
        }

        bestDifferenceMatch->AddChildByArchetype(node, {});

        matched = matched.Union(bestDifferenceMatch->GetArchetype());
    }

    _archetypeToNodeMap[type.GetID()] = node;
    return node;
}

void EntityListGraph::RemoveArchetype(const Archetype& type)
{
    assert(_archetypeToNodeMap.contains(type.GetID()));

    Node* node = _archetypeToNodeMap[type.GetID()];
    _archetypeToNodeMap.erase(type.GetID());

    for (Node* parent : node->Parents)
    {
        for (Node* child : node->Children)
        {
            if (child->GetArchetype().IsSubsetOf(parent->GetArchetype()))
            {
                parent->AddChild(child, {});
            }
        }
    }

    _nodes.Remove(*node);
}

void EntityListGraph::Query(ECSQuery& query, const Archetype& archetype)
{
    query.Clear({});

    Node* archetypeNode = _root;
    const auto it = _archetypeToNodeMap.find(archetype.GetID());
    if (it != _archetypeToNodeMap.end())
    {
        archetypeNode = it->second;
    }

    Traverse(archetypeNode, [&](Node* node)
    {
        if (!archetype.IsSubsetOf(node->GetArchetype()))
        {
            return node == _root;
        }

        query.AddEntityList(&node->EntityList, {});
        return true;
    });
}

bool EntityListGraph::Traverse(Node* root, const std::function<bool(Node*)>& callback)
{
    if (!callback(root))
    {
        return false;
    }

    for (Node* child : root->Children)
    {
        Traverse(child, callback);
    }

    return true;
}

EntityListGraph::EntityListResult EntityListGraph::GetOrCreateEntityListFor(const Archetype& type)
{
    const auto it = _archetypeToNodeMap.find(type.GetID());
    if (it != _archetypeToNodeMap.end())
    {
        return {&it->second->EntityList, false};
    }

    return {&AddArchetype(type)->EntityList, true};
}

EntityListGraph::Node* EntityListGraph::FindBestMatch(const Archetype& type)
{
    std::pair<uint32, Node*> bestMatch = {0, nullptr};
    Traverse(_root, [&bestMatch, &type](Node* root)
    {
        uint32 intersectCount = root->GetArchetype().StrictSubsetIntersectionSize(type);
        if (intersectCount > bestMatch.first)
        {
            bestMatch = {intersectCount, root};
        }

        return true;
    });

    return bestMatch.second;
}
