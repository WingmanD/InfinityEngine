#pragma once

#include "Core.h"
#include "UniformGrid2D.h"
#include <vector>
#include <algorithm>
#include <ranges>
#include <functional>
#include <map>

template <typename T>
class HitTestGrid
{
public:
    HitTestGrid(float cellSize, float width, float height, const Vector2& offset) :
        _grid(cellSize, static_cast<int16>(std::ceil(width / cellSize)), static_cast<int16>(std::ceil(height / cellSize)), offset)
    {
    }

    void InsertElement(T element, const BoundingBox2D& aabb, const std::function<bool(const T& lhs, const T& rhs)>& comparator)
    {
        if (_elementToCell.contains(element))
        {
            RemoveElement(element);  
        }
        
        _grid.ForEachCellInBox(aabb,
       [&](GridCell& cell)
       {
           auto index = std::lower_bound(cell.Elements.begin(), cell.Elements.end(), element, comparator);
           cell.Elements.insert(index, element);

           _elementToCell[element].push_back(&cell);

           return true;
       });
    }

    bool RemoveElement(T element)
    {
        auto cellIt = _elementToCell.find(element);
        if (cellIt == _elementToCell.end())
        {
            return false;
        }
        
        for (GridCell* cell : cellIt->second)
        {
            if (cell->Elements.empty())
            {
                continue;
            } 
            
            for (int64 i = static_cast<int64>(cell->Elements.size()) - 1; i >= 0; --i)
            {
                if (cell->Elements[i] == element)
                {
                    cell->Elements.erase(cell->Elements.begin() + i);
                    break;
                }
            }
        }

        return false;
    }

    T* FindAtByPredicate(const Vector2& positionWS, const std::function<bool(const Vector2&, const T&)>& predicate)
    {
        GridCell& cell = _grid.GetCellAt(positionWS);

        for (T& element : cell.Elements)
        {
            if (predicate(positionWS, element))
            {
                return &element;
            }
        }

        return nullptr;
    }

private:
    struct GridCell
    {
        std::vector<T> Elements;
    };
    
    UniformGrid2D<GridCell> _grid;

    std::map<T, std::vector<GridCell*>> _elementToCell;
};
