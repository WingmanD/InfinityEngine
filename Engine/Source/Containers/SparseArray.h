#pragma once

#include <unordered_map>

template <typename T>
class SparseArray
{
public:
    SparseArray() = default;

    bool Exists(size_t index) const
    {
        return _data.contains(index);
    }

    T* GetIfExists(size_t index)
    {
        auto it = _data.find(index);
        if (it == _data.end())
        {
            return nullptr;
        }

        return &it->second;
    }

    T& operator[](size_t index)
    {
        return _data[index];
    }

    const T& operator[](size_t index) const
    {
        return _data[index];
    }

    size_t Count() const
    {
        return _data.size();
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

    void Remove(size_t index)
    {
        _data.erase(index);
    }

    void Clear()
    {
        _data.clear();
    }

private:
    std::unordered_map<size_t, T> _data;
};
