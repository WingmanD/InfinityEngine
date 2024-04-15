#pragma once

#include "DArray.h"
#include <cassert>
#include <functional>
#include <memory>

class BucketArrayBase
{
};

// todo large allocation should happen on another thread when BucketArray is 75% full - we need to keep track of size
// todo spinlock for multithreading
template <typename T, size_t BucketSize = 100>
class BucketArray : public BucketArrayBase
{
public:
    using type = T;
    
public:
    BucketArray() = default;

    T* Add(const T& element)
    {
        T* ptr = AddUninitialized();
        assert(ptr != nullptr);

        std::construct_at(ptr, element);
        ptr->SetValid(true);

        return ptr;
    }

    T* AddDefault()
    {
        T* ptr = AddUninitialized();
        assert(ptr != nullptr);

        std::construct_at<T>(ptr);
        ptr->SetValid(true);

        return ptr;
    }

    T* AddUninitialized()
    {
        if (!_availableBucketIndices.IsEmpty())
        {
            const size_t index = _availableBucketIndices.Back();

            T* ptr = _buckets[index]->AddUninitialized();

            if (_buckets[index]->IsFull())
            {
                _availableBucketIndices.PopBack();
            }

            return ptr;
        }

        _buckets.Add(std::make_unique<Bucket>());
        _availableBucketIndices.Add(_buckets.Count() - 1);

        return _buckets.Back()->AddUninitialized();
    }

    template <typename... Args>
    T* Emplace(Args&&... args)
    {
        T* ptr = AddUninitialized();
        assert(ptr != nullptr);

        std::construct_at(ptr, std::forward<Args>(args)...);
        ptr->SetValid(true);

        return ptr;
    }

    bool Remove(T& element)
    {
        for (auto i = 0; i < _buckets.Count(); ++i)
        {
            auto& bucket = _buckets[i];

            const bool wasFull = bucket->IsFull();
            if (bucket->Remove(element))
            {
                if (wasFull)
                {
                    _availableBucketIndices.Add(i);
                }

                return true;
            }
        }

        return false;
    }

    void ForEach(const std::function<bool(T&)>& callback)
    {
        for (auto& bucket : _buckets)
        {
            for (size_t i = 0; i < BucketSize; ++i)
            {
                T& element = (*bucket.get())[i];
                if (!element.IsValid())
                {
                    continue;
                }
                
                if (!callback(element))
                {
                    return;
                }
            }
        }
    }

    void ForEach(const std::function<bool(const T&)>& callback) const
    {
        for (auto& bucket : _buckets)
        {
            for (size_t i = 0; i < BucketSize; ++i)
            {
                T& element = (*bucket.get())[i];
                if (!element.IsValid())
                {
                    continue;
                }

                if (!callback(element))
                {
                    return;
                }
            }
        }
    }

    void Clear()
    {
        _buckets.Clear();
        _availableBucketIndices.Clear();
    }

private:
    struct Bucket
    {
    public:
        explicit Bucket() = default;

        ~Bucket()
        {
            for (size_t i = 0; i < BucketSize; ++i)
            {
                T& element = GetElement(i);
                if (!element.IsValid())
                {
                    continue;
                }

                element.SetValid(false);
                element.~T();
            }
        }

        T* AddUninitialized()
        {
            T* ptr;
            if (!_freeIndices.IsEmpty())
            {
                const size_t index = _freeIndices.Back();
                _freeIndices.PopBack();

                ptr = &reinterpret_cast<T*>(&_data)[index];
            }
            else
            {
                ptr = &reinterpret_cast<T*>(&_data)[_index++];
            }

            return ptr;
        }

        bool Remove(T& element)
        {
            const size_t index = IndexOf(element);
            if (index < BucketSize)
            {
                element.SetValid(false);
                element.~T();

                _freeIndices.Add(index);

                return true;
            }

            return false;
        }

        size_t IndexOf(const T& element) const
        {
            const size_t index = std::addressof(element) - &reinterpret_cast<const T*>(_data)[0];
            return index;
        }

        bool Contains(const T& element) const
        {
            return IndexOf(element) < BucketSize;
        }

        bool IsFull() const
        {
            return _index == BucketSize;
        }

        T& GetElement(size_t index)
        {
            return reinterpret_cast<T*>(&_data)[index];
        }

        T& operator[](size_t index)
        {
            return GetElement(index);
        }

    private:
        alignas(T) std::byte _data[sizeof(T) * BucketSize]{};
        size_t _index = 0;

        DArray<size_t, BucketSize / 2> _freeIndices{};
    };

    DArray<std::unique_ptr<Bucket>, 16> _buckets{};
    DArray<size_t, 8> _availableBucketIndices{};
};
