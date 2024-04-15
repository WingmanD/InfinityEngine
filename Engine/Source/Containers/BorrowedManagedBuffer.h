#pragma once

#include "DArray.h"

template <typename T>
class BorrowedManagedBuffer
{
public:
    BorrowedManagedBuffer(T* data, size_t capacity, size_t count = 0)
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

    size_t Add(const T& data)
    {
        _data[_count] = data;
        ++_count;

        return _count;
    }

    size_t Emplace(T&& data)
    {
        _data[_count] = std::move(data);
        return _count++;
    }

    void RemoveAtSwap(size_t index)
    {
        _data[index] = _data[_count - 1];
        --_count;
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

    void Initialize(T* data, size_t capacity, size_t count)
    {
        _data = data;
        _count = count;
        _capacity = capacity;
    }

    T* GetData()
    {
        return _data;
    }

    size_t Count() const
    {
        return _count;
    }

    size_t Capacity() const
    {
        return _capacity;
    }

    const DArray<size_t>& GetDirtyIndices() const
    {
        return _dirtyIndices;
    }

    void ClearDirtyIndices()
    {
        _dirtyIndices.Clear();
    }

private:
    T* _data = nullptr;
    size_t _count = 0;
    size_t _capacity = 0;

    DArray<size_t> _dirtyIndices;

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
