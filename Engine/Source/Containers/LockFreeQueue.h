#pragma once

#include <atomic>

template <typename T>
class LockFreeQueue
{
public:
    LockFreeQueue() : _head(nullptr), _tail(nullptr)
    {
    }

    void Enqueue(T&& value)
    {
        Node* newNode = new Node(std::move(value), nullptr);
        Node* prevTail = _tail.exchange(newNode, std::memory_order_acq_rel);
        if (prevTail != nullptr)
        {
            prevTail->Next = newNode;
        }
        else
        {
            _head = newNode;
        }
    }

    bool Dequeue(T& value)
    {
        Node* currentHead = _head.load(std::memory_order_acquire);
        if (currentHead == nullptr)
        {
            return false;
        }

        _head = currentHead->Next;
        if (currentHead->Next == nullptr)
        {
            _tail.store(nullptr, std::memory_order_release);
        }
        value = std::move(currentHead->Data);
        delete currentHead;
        return true;
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return _head.load(std::memory_order_acquire) == nullptr;
    }

private:
    struct Node
    {
        T Data;
        Node* Next;
    };

    std::atomic<Node*> _head;
    std::atomic<Node*> _tail;
};
