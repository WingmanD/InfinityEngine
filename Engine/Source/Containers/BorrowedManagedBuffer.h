#pragma once

#include "DArray.h"
#include "Type.h"

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
        if constexpr (IsReflectedType<T>)
        {
            const Object& object = dynamic_cast<const Object&>(data);
            const size_t dataOffset = object.GetType()->GetDataOffset();

            memcpy(reinterpret_cast<std::byte*>(_data) + _count * _elementSize,
                   &object + dataOffset,
                   _elementSize);
        }
        else
        {
            (*this)[_count] = data;
        }
        
        _dirtyIndices.Add(_count);
        
        return _count++;
    }

    uint32 AddDefault()
    {
        if constexpr (IsReflectedType<T>)
        {
            const Object* object = _type->GetCDO();

            std::byte* dest = reinterpret_cast<std::byte*>(_data) + _count * _elementSize;
            const std::byte* src = reinterpret_cast<const std::byte*>(object) + _elementDataOffset;
            memcpy(dest, src, _elementSize);
        }
        else
        {
            (*this)[_count] = T();
        }
        
        _dirtyIndices.Add(_count);
        
        return _count++;
    }

    uint32 Emplace(T&& data)
    {
        (*this)[_count] = std::move(data);
        return _count++;
    }

    void RemoveAtSwap(uint32 index)
    {
        (*this)[index] = (*this)[_count - 1];
        --_count;
    }

    T& operator[](uint32 index)
    {
        _dirtyIndices.Add(index);
        
        if constexpr (IsReflectedType<T>)
        {
            const int64 offset = index * _elementSize - static_cast<int64>(_elementDataOffset);
            return reinterpret_cast<T&>(*(reinterpret_cast<std::byte*>(_data) + offset));
        }
        else
        {
            return _data[index];
        }
    }

    const T& operator[](uint32 index) const
    {
        if constexpr (IsReflectedType<T>)
        {
            const int64 offset = index * _elementSize - _elementDataOffset;
            return reinterpret_cast<T&>(*(reinterpret_cast<std::byte*>(_data) + offset));
        }
        else
        {
            return _data[index];
        }
    }

    void Initialize(T* data, uint32 capacity, uint32 count, const Type* type = nullptr)
    {
        _data = data;
        _count = count;
        _capacity = capacity;
        _type = type;

        if constexpr (IsReflectedType<T>)
        {
            if (_type != nullptr)
            {
                _elementDataOffset = static_cast<uint32>(_type->GetDataOffset());
                _elementSize = static_cast<uint32>(_type->GetSize()) - _elementDataOffset;
            }
        }
        else
        {
            _elementSize = sizeof(T);
        }
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
    uint32 _elementSize = 0;
    uint32 _elementDataOffset = 0;
    const Type* _type = nullptr;

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
