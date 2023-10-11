#include "ThreadPool.h"
#include <thread>

ThreadPool::ThreadPool() : ThreadPool(std::thread::hardware_concurrency())
{
}

ThreadPool::ThreadPool(unsigned int numThreads)
{
    _workerThreads.reserve(numThreads);

    for (unsigned int i = 0; i < numThreads; ++i)
    {
        _workerThreads.emplace_back(&ThreadPool::ThreadMain, this);
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(_queueMutex);
        _terminateRequested = true;
    }

    _wakeCondition.notify_all();

    for (auto& thread : _workerThreads)
    {
        thread.join();
    }
}

void ThreadPool::EnqueueTask(std::function<void()>&& task)
{
    {
        std::unique_lock<std::mutex> lock(_queueMutex);
        _taskQueue.push(std::move(task));
        ++_pendingTasks;
    }

    _wakeCondition.notify_one();
}

void ThreadPool::WaitForAll()
{
    std::unique_lock<std::mutex> lock(_queueMutex);
    _allTasksCompleteCondition.wait(lock, [this]()
    {
        return _pendingTasks == 0;
    });
}

void ThreadPool::ThreadMain()
{
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(_queueMutex);

            _wakeCondition.wait(lock, [this]
            {
                return !_taskQueue.empty() || _terminateRequested;
            });

            if (_terminateRequested)
            {
                return;
            }

            task = _taskQueue.front();
            _taskQueue.pop();
        }

        task();

        --_pendingTasks;
        if (_pendingTasks == 0)
        {
            _allTasksCompleteCondition.notify_all();
        }
    }
}
