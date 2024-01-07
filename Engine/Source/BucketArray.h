#pragma once

#include "Core.h"

// todo large allocation should happen on another thread when BucketArray is 75% full - we need to keep track of size
// todo indices are size_t which is unsigned, we can't represent invalid index
template <typename T, size_t BucketSize>
class BucketArray
{
public:
    BucketArray() = default;

    T* Add(const T& element)
    {
        if (!_availableBucketIndices.empty())
        {
            const size_t index = _availableBucketIndices.back();

            T* ptr = _buckets[index]->Add(element);
            
            if (_buckets[index]->IsFull())
            {
                _availableBucketIndices.pop_back();
            }
            
            return ptr;
        }
        
        _buckets.push_back(std::make_unique<Bucket>());
        _buckets.back()->Add(element);
        
        _availableBucketIndices.push_back(_buckets.size() - 1);
    }

    void Remove(const T& element)
    {
        for (auto& bucket : _buckets)
        {
            const bool wasFull = bucket->IsFull();
            if (bucket.Remove(element))
            {
                if (wasFull)
                {
                    _availableBucketIndices.push_back(_buckets.size() - 1);
                }
                
                return;
            }
        }
    }

    void ForEach(const std::function<bool(T&)>& callback)
    {
        for (auto& bucket : _buckets)
        {
            for (auto& element : bucket)
            {
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
            for (auto& element : bucket)
            {
                if (!callback(element))
                {
                    return;
                }
            }
        }
    }

    void Clear()
    {
        _buckets.clear();
    }

private:
    // todo IsValid for each element - before we used if constexpr with check if T has IsValid method
    struct Bucket
    {
    public:
        void Add(const T& element)
        {
            if (!_freeIndices.empty())
            {
                const size_t index = _freeIndices.back();
                _freeIndices.pop_back();

                _data[index] = element;
                return;
            }

            _data[_index++] = element;
        }

        bool Remove(const T& element)
        {
            const size_t index = IndexOf(element);
            if (index < BucketSize)
            {
                _freeIndices.push_back(index);
                return true;
            }

            return false;
        }

        size_t IndexOf(const T& element) const
        {
            const size_t index = std::addressof(element) - std::addressof(_data[0]);
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

        auto begin()
        {
            return _data.begin();
        }

        auto end()
        {
            return _data.begin() + BucketSize;
        }

    private:
        std::array<T, BucketSize> _data;
        size_t _index = 0;

        std::vector<size_t> _freeIndices;
    };

    std::vector<std::unique_ptr<Bucket>> _buckets;
    std::vector<size_t> _availableBucketIndices;
};
