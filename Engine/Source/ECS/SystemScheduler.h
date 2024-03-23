#pragma once

#include "CoreMinimal.h"
#include "IValidateable.h"
#include "Containers/BucketArray.h"
#include "Systems/System.h"

class SystemScheduler
{
public:
    explicit SystemScheduler();

    SystemScheduler(const SystemScheduler&) = delete;
    SystemScheduler(SystemScheduler&&) = delete;

    SystemScheduler& operator=(const SystemScheduler&) = delete;
    SystemScheduler& operator=(SystemScheduler&&) = delete;

    ~SystemScheduler() = default;

    void Initialize();
    void Tick(double deltaTime);
    void Shutdown();

    template <typename SystemType> requires IsA<SystemType, SystemBase>
    SystemType& AddSystem()
    {
        return static_cast<SystemType&>(AddSystem(std::make_unique<SystemType>()));
    }

    const DArray<std::unique_ptr<SystemBase>>& GetSystems() const;

private:
    struct Task : public IValidateable
    {
    public:
        SystemBase* System;

        DArray<Task*, 8> Parents;
        DArray<Task*, 8> Children;

        std::atomic<int32> ParentsCompleted = 0;
        bool Valid = false;

    public:
        void LinkDependentChildTask(Task* newChild);
        bool CanBeExecutedInParallelWith(const Task& rhs) const;
        std::pair<int32, Task*> FindClosestCompatibleTask(const Task& newTask);

        bool operator==(const Task& rhs) const;

        // IValidateable
    public:
        void SetValidImplementation(bool value);
        bool IsValidImplementation() const;
    };

    Task* _startTask = nullptr;
    Task* _endTask = nullptr;

    BucketArray<Task> _tasks;
    DArray<std::unique_ptr<SystemBase>> _systems;

private:
    void ForEachTask(const std::function<void(Task*)>& callback, Task* start = nullptr) const;
    void EnqueueTask(Task* task, double deltaTime);

    SystemBase& AddSystem(std::unique_ptr<SystemBase>&& system);
};
