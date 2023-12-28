#pragma once

#include "Core.h"
#include "BoundingBox2D.h"
#include <vector>

class BoundingBox2D;

template <typename T>
class UniformGrid2D
{
public:
    struct Index2D
    {
        int16 X = 0;
        int16 Y = 0;
    };

public:
    UniformGrid2D(float cellSize, int16 width, int16 height, const Vector2& offset = Vector2::Zero) :
        _cellSize(cellSize),
        _width(width),
        _height(height),
        _offset(offset)
    {
        _grid.resize(width * height);
    }

    T& GetCell(Index2D index)
    {
        const int64 index1D = static_cast<int64>(index.X + index.Y * _cellSize);
        assert(index1D >= 0 && index1D < static_cast<int64>(_grid.size()));
        return _grid[index1D];
    }

    void ForEachCellInBox(const BoundingBox2D& aabb, const std::function<bool(T&)>& callback)
    {
        const Index2D minIndex = GetCellIndex(aabb.GetMin());
        const Index2D maxIndex = GetCellIndex(aabb.GetMax());

        for (int16 x = std::max<int16>(0, minIndex.X); x < std::min(maxIndex.X, _width); ++x)
        {
            for (int16 y = std::max<int16>(0, minIndex.Y); y < std::min(maxIndex.Y, _height); ++y)
            {
                if (!callback(GetCell({x, y})))
                {
                    return;
                }
            }
        }
    }

    Index2D GetCellIndex(const Vector2& positionWS)
    {
        const Vector2 position = positionWS + _offset;
        
        const int16 x = static_cast<int16>(position.x / _cellSize);
        const int16 y = static_cast<int16>(position.y / _cellSize);

        return {x, y};
    }

    T& GetCellAt(const Vector2& positionWS)
    {
        return GetCell(GetCellIndex(positionWS));
    }

    float GetCellSize() const
    {
        return _cellSize;
    }

    int16 GetHeight() const
    {
        return _height;
    }

    int16 GetWidth() const
    {
        return _width;
    }

private:
    std::vector<T> _grid;

    float _cellSize = 0.0f;
    int16 _width = 0;
    int16 _height = 0;

    Vector2 _offset = Vector2::Zero;
};
