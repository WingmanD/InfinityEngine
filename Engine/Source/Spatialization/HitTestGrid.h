#pragma once

#include "Core.h"
#include "UniformGrid2D.h"

template <typename T>
class HitTestGrid
{
public:
    HitTestGrid(float cellSize, float width, float height, const Vector2& offset) :
        _grid(cellSize, static_cast<int16>(std::ceil(width / cellSize)), static_cast<int16>(std::ceil(height / cellSize)), offset)
    {
    }

    void InsertElement(const T& element, const BoundingBox2D& aabb, const std::function<bool(const T& lhs, const T& rhs)>& comparator)
    {
        _grid.ForEachCellInBox(aabb,
       [&](GridCell& cell)
       {
           auto index = std::lower_bound(cell.Elements.begin(), cell.Elements.end(), element, comparator);
           
           if (std::find(cell.Elements.begin(), cell.Elements.end(), element) != cell.Elements.end())
           {
               return true;
           }
           
           cell.Elements.insert(index, element);

           return true;
       });
    }

    bool RemoveElement(const T& element, const BoundingBox2D& aabb, const std::function<bool(const T& lhs, const T& rhs)>& comparator)
    {
        bool removed = false;

        _grid.ForEachCellInBox(aabb,
        [&](GridCell& cell)
        {
            auto index = std::lower_bound(cell.Elements.begin(), cell.Elements.end(), element, comparator);
            if (index != cell.Elements.end() && *index == element)
            {
                cell.Elements.erase(index);
                removed = true;
            }

            return true;
        });

        return removed;
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
};
