#pragma once

#include "moodycamel/concurrentqueue.h"

/**
 * A lock-free MPMC queue.
 * This is a wrapper around moodycamel::ConcurrentQueue (https://github.com/cameron314/concurrentqueue).
 * Licensed under the Simplified BSD License (BSD 2-Clause License).
 * For full license text of the moodycamel::ConcurrentQueue, visit
 * https://github.com/cameron314/concurrentqueue/blob/master/LICENSE.md
 */
template <typename T>
class LockFreeQueue
{
public:
    LockFreeQueue() = default;

    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue(LockFreeQueue&&) = delete;

    LockFreeQueue& operator=(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(LockFreeQueue&&) = delete;

    ~LockFreeQueue() = default;

    bool Enqueue(T&& value)
    {
        return _queue.enqueue(value);
    }

    [[nodiscard]] bool Dequeue(T& value)
    {
        return _queue.try_dequeue(value);
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return _queue.size_approx() == 0;
    }

private:
    moodycamel::ConcurrentQueue<T> _queue{};
};
