#include "SystemScheduler.h"
#include "Engine/Engine.h"
#include <queue>

SystemScheduler::SystemScheduler()
{
    _startTask = _tasks.AddDefault();

    _endTask = _tasks.AddDefault();
    _startTask->Children.Add(_endTask);
    _endTask->Parents.Add(_startTask);
}

void SystemScheduler::Initialize()
{
}

void SystemScheduler::Tick(double deltaTime)
{
    if (_startTask->Children.Count() == 1 && _startTask->Children[0] == _endTask)
    {
        return;
    }

    _endTask->ParentsCompleted.store(0, std::memory_order_release);
    _endTask->IsFinished.store(false, std::memory_order_release);

    for (Task* task : _startTask->Children)
    {
        EnqueueTask(task, deltaTime);
    }

    _endTask->IsFinished.wait(false, std::memory_order_acquire);
}

void SystemScheduler::Shutdown()
{
    for (const std::unique_ptr<SystemBase>& system : _systems)
    {
        system->CallShutdown({});
    }
}

const DArray<std::unique_ptr<SystemBase>>& SystemScheduler::GetSystems() const
{
    return _systems;
}

SystemBase& SystemScheduler::AddSystem(std::unique_ptr<SystemBase>&& system)
{
    Task* newTask = _tasks.AddDefault();
    newTask->System = system.get();
    _systems.Add(std::move(system));

    Task* bestTask = _endTask;
    const std::pair<int32, Task*> result = _startTask->FindClosestCompatibleTask(*newTask);
    if (result.second != nullptr)
    {
        bestTask = result.second;
    }

    if (bestTask == _endTask)
    {
        bool found = false;
        for (Task* parent : _endTask->Parents)
        {
            if (!newTask->CanBeExecutedInParallelWith(*parent))
            {
                parent->Children.RemoveSwap(_endTask);
                _endTask->Parents.RemoveSwap(parent);
                
                parent->Children.Add(newTask);
                newTask->Parents.Add(parent);
                
                newTask->Children.Add(_endTask);
                _endTask->Parents.Add(newTask);

                found = true;
                break;
            }
        }

        // todo this doesn't seem to work, implement a debug print to visualize the graph

        if (!found)
        {
            newTask->Children.Add(_endTask);
        }
    }
    else
    {
        bestTask->LinkDependentChildTask(newTask);
    }

    newTask->System->CallInitialize({});

    return *newTask->System;
}

void SystemScheduler::Task::LinkDependentChildTask(Task* newChild)
{
    newChild->Parents.Add(this);

    for (Task* childTask : Children)
    {
        for (const Task* grandChild : childTask->Children)
        {
            if (!newChild->CanBeExecutedInParallelWith(*grandChild))
            {
                childTask->Parents.Add(newChild);
                break; // todo is this ok?
            }
        }
    }

    Children.Add(newChild);
}

bool SystemScheduler::Task::CanBeExecutedInParallelWith(const Task& rhs) const
{
    if (System == nullptr || rhs.System == nullptr)
    {
        return false;
    }

    return System->GetArchetype().CanBeExecutedInParallelWith(System->GetArchetype());
}

std::pair<int32, SystemScheduler::Task*> SystemScheduler::Task::FindClosestCompatibleTask(const Task& newTask)
{
    std::pair<int32, Task*> bestMatch = {std::numeric_limits<int32>::max(), nullptr};
    bool canBeExecutedInParallelWithAllChildren = true;

    for (Task* child : Children)
    {
        if (!child->CanBeExecutedInParallelWith(newTask))
        {
            canBeExecutedInParallelWithAllChildren = false;

            const std::pair<int32, Task*> result = child->FindClosestCompatibleTask(newTask);
            if (result.first < bestMatch.first)
            {
                bestMatch = result;
            }
        }
    }

    if (canBeExecutedInParallelWithAllChildren)
    {
        return {0, this};
    }

    return bestMatch;
}

bool SystemScheduler::Task::operator==(const Task& rhs) const
{
    return reinterpret_cast<uintptr_t>(this) == reinterpret_cast<uintptr_t>(&rhs);
}

void SystemScheduler::Task::SetValidImplementation(bool value)
{
    Valid = value;
}

bool SystemScheduler::Task::IsValidImplementation() const
{
    return Valid;
}

void SystemScheduler::ForEachTask(const std::function<void(Task*)>& callback, Task* start /*= nullptr*/) const
{
    if (start == nullptr)
    {
        start = _startTask;
    }
    else
    {
        callback(start);
    }

    std::queue<Task*> queue;

    for (Task* child : start->Children)
    {
        queue.push(child);
    }

    while (!queue.empty())
    {
        Task* task = queue.front();
        queue.pop();

        callback(task);

        for (Task* child : task->Children)
        {
            queue.push(child);
        }
    }
}

void SystemScheduler::EnqueueTask(Task* task, double deltaTime)
{
    task->ParentsCompleted.store(0, std::memory_order_release);
    task->IsFinished.store(false, std::memory_order_release);

    Engine::Get().GetThreadPool().EnqueueTask([task, deltaTime, this]()
    {
        task->System->CallTick(deltaTime, {});

        for (Task* child : task->Children)
        {
            const int32 total = child->ParentsCompleted.fetch_add(1, std::memory_order_acq_rel) + 1;
            if (total == child->Parents.Count())
            {
                if (child == _endTask)
                {
                    child->IsFinished.store(true, std::memory_order_release);
                    child->IsFinished.notify_one();
                }
                else
                {
                    EnqueueTask(child, deltaTime);
                }
            }
        }
    });
}
