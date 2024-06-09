#pragma once

#include "FNV1a.h"
#include "SparseArray.h"
#include "Math/Math.h"

struct Index3D
{
    int64 X;
    int64 Y;
    int64 Z;
};

template <typename T>
class SparseUniformGrid3D
{
public:
    [[nodiscard]] bool Exists(int64 x, int64 y, int64 z) const
    {
        return _data.Exists(Hash(x, y, z));
    }

    T* GetIfExists(int64 x, int64 y, int64 z)
    {
        return _data.GetIfExists(Hash(x, y, z));
    }

    T& operator[](uint64 index)
    {
        return _data[index];
    }

    const T& operator[](uint64 index) const
    {
        return _data[index];
    }

    T& operator[](const Index3D& index)
    {
        return _data[Hash(index.X, index.Y, index.Z)];
    }

    const T& operator[](const Index3D& index) const
    {
        return _data[Hash(index.X, index.Y, index.Z)];
    }

    // T& operator[](int64 x, int64 y, int64 z)
    // {
    //     return _data[Hash(x, y, z)];
    // }
    //
    // const T& operator[](int64 x, int64 y, int64 z) const
    // {
    //     return _data[Hash(x, y, z)];
    // }

    [[nodiscard]] int64 Count() const
    {
        return _data.Count();
    }

    auto begin()
    {
        return _data.begin();
    }

    auto end()
    {
        return _data.end();
    }

    auto begin() const
    {
        return _data.begin();
    }

    auto end() const
    {
        return _data.end();
    }

    void ForEachCellInBox(const Index3D& min, const Index3D& max, std::function<bool(const Index3D&, T&)> func)
    {
        const Index3D actualMin = {Math::Min(min.X, max.X), Math::Min(min.Y, max.Y), Math::Min(min.Z, max.Z)};
        const Index3D actualMax = {Math::Max(min.X, max.X), Math::Max(min.Y, max.Y), Math::Max(min.Z, max.Z)};
        
        for (int64 x = actualMin.X; x <= actualMax.X; ++x)
        {
            for (int64 y = actualMin.Y; y <= actualMax.Y; ++y)
            {
                for (int64 z = actualMin.Z; z <= actualMax.Z; ++z)
                {
                    T* cell = GetIfExists(x, y, z);
                    if (cell == nullptr)
                    {
                        continue;
                    }
                    
                    if (!func({x, y, z}, *cell))
                    {
                        return;
                    }
                }
            }
        }
    }

    void Remove(int64 x, int64 y, int64 z)
    {
        _data.Remove(Hash(x, y, z));
    }

    void Clear()
    {
        _data.Clear();
    }

private:
    SparseArray<T> _data;

private:
    static constexpr int64 Hash(int64 x, int64 y, int64 z)
    {
        FNV1a fnv;
        fnv.Combine(x);
        fnv.Combine(y);
        fnv.Combine(z);
        return static_cast<int64>(fnv.GetHash());
    }
};
