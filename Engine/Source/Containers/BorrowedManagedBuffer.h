#pragma once

#include "DArray.h"

template <typename T>
class BorrowedManagedBuffer
{
public:
    BorrowedManagedBuffer(T* data, uint32 capacity, uint32 count = 0)
    {
        Initialize(data, capacity, count);
    }

    BorrowedManagedBuffer(const BorrowedManagedBuffer& other)
    {
        _data = other._data;
        _count = other._count;
        _capacity = other._capacity;
    }

    BorrowedManagedBuffer(BorrowedManagedBuffer&& other) noexcept
    {
        Swap(other);
    }

    BorrowedManagedBuffer& operator=(const BorrowedManagedBuffer& other)
    {
        if (this != &other)
        {
            _data = other._data;
            _count = other._count;
            _capacity = other._capacity;
        }

        return *this;
    }

    BorrowedManagedBuffer& operator=(BorrowedManagedBuffer&& other) noexcept
    {
        if (this != &other)
        {
            Swap(other);
        }

        return *this;
    }

    ~BorrowedManagedBuffer() = default;

    uint32 Add(const T& data)
    {
        _data[_count] = data;
        ++_count;

        return _count;
    }

    uint32 Emplace(T&& data)
    {
        _data[_count] = std::move(data);
        return _count++;
    }

    void RemoveAtSwap(uint32 index)
    {
        _data[index] = _data[_count - 1];
        --_count;
    }

    T& operator[](uint32 index)
    {
        _dirtyIndices.Add(index);
        return _data[index];
    }

    const T& operator[](uint32 index) const
    {
        return _data[index];
    }

    void Initialize(T* data, uint32 capacity, uint32 count)
    {
        _data = data;
        _count = count;
        _capacity = capacity;
    }

    T* GetData()
    {
        return _data;
    }

    uint32 Count() const
    {
        return _count;
    }

    uint32 Capacity() const
    {
        return _capacity;
    }

    const DArray<uint32>& GetDirtyIndices() const
    {
        return _dirtyIndices;
    }

    void ClearDirtyIndices()
    {
        _dirtyIndices.Clear();
    }

private:
    T* _data = nullptr;
    uint32 _count = 0;
    uint32 _capacity = 0;

    DArray<uint32> _dirtyIndices;

private:
    void Swap(BorrowedManagedBuffer& other) noexcept
    {
        _data = other._data;
        other._data = nullptr;

        _count = other._count;
        other._count = 0;

        _capacity = other._capacity;
        other._capacity = 0;
    }
};
