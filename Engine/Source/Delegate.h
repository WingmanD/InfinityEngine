#pragma once

#include "Core.h"
#include <functional>
#include <vector>

struct DelegateHandle
{
public:
    uint64 Index = 0;

public:
    bool IsValid() const
    {
        return Index != 0;
    }
};

template <typename... Args>
class Delegate
{
public:
    DelegateHandle Add(std::function<void(Args&&...)>&& function)
    {
        _functions.push_back(function);

        return {_functions.size() - 1};
    }

    void Remove(const DelegateHandle& handle)
    {
        _functions.erase(_functions.begin() + handle.Index);
    }

    void Broadcast(Args... args)
    {
        for (auto& function : _functions)
        {
            function(std::forward<Args>(args)...);
        }
    }

    void Clear()
    {
        _functions.clear();
    }

private:
    std::vector<std::function<void(Args...)>> _functions;
};
