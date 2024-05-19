#include "EntityListGraph.h"
#include "Archetype.h"

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
    _archetypeToNodeMap[type.GetID()] = node;

    Node* bestMatchNode = FindBestMatch(type);
    if (bestMatchNode == nullptr)
    {
        _root->AddChildByArchetype(node, {});
        return node;
    }

    if (bestMatchNode->GetArchetype().IsSupersetOf(type))
    {
        for (int64 i = bestMatchNode->Parents.Count() - 1; i >= 0; --i)
        {
            Node* parent = bestMatchNode->Parents[i];
            
            if (parent->GetArchetype().IsSubsetOf(type))
            {
                parent->AddChildByArchetype(node, {});;
            }
        }
    }
    else if (type.IsSupersetOf(bestMatchNode->GetArchetype()))
    {
        bestMatchNode->AddChildByArchetype(node, {});
    }

    if (type == bestMatchNode->GetArchetype())
    {
        return node;
    }

    Archetype matched = bestMatchNode->GetArchetype().Intersection(type);

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
            _root->AddChildByArchetype(node, {});
            
            break;
        }
        
        matched = matched.Union(bestDifferenceMatch->GetArchetype());
    }

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

void EntityListGraph::Query(ECSQuery& query, const Archetype& archetype) const
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

        if (node->EntityList.GetArchetype().IsValid())
        {
            query.AddEntityList(&node->EntityList, {});
        }

        return true;
    });
}

bool EntityListGraph::Traverse(Node* root, const std::function<bool(Node*)>& callback) const
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

    const EntityListResult result =  {&AddArchetype(type)->EntityList, true};
    
    return result;
}

void EntityListGraph::LogGraph(const Node* root) const
{
    for (const Node* child : root->Children)
    {
        LOG(L"{} -> {}", root->EntityList.GetArchetype(), child->GetArchetype());
        LogGraph(child);
    } 
}

EntityListGraph::Node* EntityListGraph::FindBestMatch(const Archetype& type) const
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
