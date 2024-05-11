#pragma once

#include "NonCopyable.h"
#include <atomic>

class SpinLock
{
public:
    void Lock()
    {
        while (_flag.test_and_set(std::memory_order_acquire))
        {
            while (!_flag.test());
        }
    }

    void Unlock()
    {
        _flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag _flag = ATOMIC_FLAG_INIT;
};

class SpinLockGuard final : public NonCopyable<SpinLockGuard>
{
public:
    SpinLockGuard(SpinLock& lock) : _lock(&lock)
    {
        _lock->Lock();
    }
    
    SpinLockGuard(SpinLockGuard&&) = delete;
    SpinLockGuard& operator=(SpinLockGuard&&) = delete;

    ~SpinLockGuard()
    {
        _lock->Unlock();
    }

private:
    SpinLock* _lock;
};
