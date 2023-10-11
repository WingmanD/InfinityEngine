#pragma once

#include <functional>
#include <condition_variable>
#include <queue>

class ThreadPool {
public:
    ThreadPool();
    explicit ThreadPool(unsigned int numThreads);
    
    ThreadPool(const ThreadPool& other) = delete;
    ThreadPool(ThreadPool&& other) = delete;
    ThreadPool& operator=(const ThreadPool& other) = delete;
    ThreadPool& operator=(ThreadPool&& other) = delete;
    
    ~ThreadPool();

    void EnqueueTask(std::function<void()> &&task);
    void WaitForAll();

private:
    std::vector<std::thread> _workerThreads;
    std::queue<std::function<void()>> _taskQueue;
    std::atomic<unsigned int> _pendingTasks = 0;

    std::mutex _queueMutex;
    std::condition_variable _wakeCondition;
    std::condition_variable _allTasksCompleteCondition;

    bool _terminateRequested = false;

private:
    void ThreadMain();
};
