#pragma once

#include "LockFreeQueue.h"
#include <functional>

template <typename OwnerType>
class EventQueue : public LockFreeQueue<std::function<void(OwnerType*)>>
{
public:
    EventQueue(OwnerType* owner) : _owner(owner)
    {
    }

    void ProcessEvents()
    {
        std::function<void(OwnerType*)> event;

        while (this->Dequeue(event))
        {
            event(GetOwner());
        }
    }

    OwnerType* GetOwner() const
    {
        return _owner;
    }
    
private:
    OwnerType* _owner;
};
