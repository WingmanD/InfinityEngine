#pragma once

#include "Core.h"
#include <functional>
#include <map>
#include <vector>

#include "IDGenerator.h"

struct DelegateHandle
{
public:
    uint64 ID = 0;

public:
    bool IsValid() const
    {
        return ID != 0;
    }
};

template <typename... Args>
class Delegate
{
public:
    DelegateHandle Add(std::function<void(Args...)>&& function)
    {
        const uint64 id = _idGenerator.GenerateID();
        _functions.push_back({function, id});

        _handleIDToIndex[id] = _functions.size() - 1;

        return {id};
    }

    void Remove(const DelegateHandle& handle)
    {
        const auto it = _handleIDToIndex.find(handle.ID);
        if (it == _handleIDToIndex.end())
        {
            return;
        }

        size_t index = it->second;
        _handleIDToIndex.erase(handle.ID);

        if (index == _functions.size() - 1)
        {
            _functions.pop_back();
            return;
        }
        
        _functions[index] = _functions.back();
        _functions.pop_back();

        _handleIDToIndex[_functions[index].ID] = index;
    }

    void Broadcast(Args... args)
    {
        for (auto& function : _functions)
        {
            function.Function(std::forward<Args>(args)...);
        }
    }

    void Clear()
    {
        _functions.clear();
    }

private:
    struct FunctionWrapper
    {
        std::function<void(Args...)> Function;
        uint64 ID = 0;
    };

    std::vector<FunctionWrapper> _functions;
    std::map<uint64, size_t> _handleIDToIndex;

    IDGenerator _idGenerator;
};
