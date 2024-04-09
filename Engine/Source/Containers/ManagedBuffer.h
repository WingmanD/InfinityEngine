#pragma once

#include "DArray.h"

template <typename T>
class ManagedBuffer
{
public:
    ManagedBuffer()
    {
        _data.Reserve(16);
    }

    ManagedBuffer(const ManagedBuffer&) = delete;
    ManagedBuffer(ManagedBuffer&& other) noexcept = delete;

    ManagedBuffer& operator=(const ManagedBuffer&) = delete;
    ManagedBuffer& operator=(ManagedBuffer&& other) noexcept = delete;

    ~ManagedBuffer() = default;

    size_t Add(const T& data)
    {
        _data.Add(data);

        const size_t index = _data.Count() - 1;
        _dirtyIndices.Add(index);

        return index;
    }

    size_t Emplace(T&& data)
    {
        _data.Emplace(std::move(data));

        const size_t index = _data.Count() - 1;
        _dirtyIndices.Add(index);

        return index;
    }

    void RemoveAtSwap(size_t index)
    {
        _data.RemoveAtSwap(index);
        _dirtyIndices.Add(index);
    }

    T& operator[](size_t index)
    {
        _dirtyIndices.Add(index);
        return _data[index];
    }

    const T& operator[](size_t index) const
    {
        return _data[index];
    }

    T* GetData()
    {
        return _data.GetData();
    }

    void ClearDirtyIndices()
    {
        _dirtyIndices.Clear();
    }

    const DArray<size_t>& GetDirtyIndices() const
    {
        return _dirtyIndices;
    }

    size_t Count() const
    {
        return _data.Count();
    }

    size_t Capacity() const
    {
        return _data.Capacity();
    }

private:
    DArray<T> _data;
    DArray<size_t> _dirtyIndices;
};
