#pragma once

#include "task.h"

#include "logger.h"



template<typename TId, typename TFunc>
class Manager {
public:
    using TaskFunction = TFunc;
    using TaskID = TId;
    using Task = TaskImpl<TaskID, TaskFunction>;
    using TaskReturnValue = std::invoke_result_t<TaskFunction>;
    using TaskId = typename Task::TaskId;


    Manager() = default;

    virtual bool AddTask(const Task& task) = 0;
    virtual std::optional<Task> GetTask() = 0;
    virtual bool HasTask() = 0;
    virtual TaskReturnValue CalculateTask(const Task& task) = 0;
    virtual void SaveResult(TaskId id, const TaskReturnValue& value) = 0;
    virtual std::optional<TaskReturnValue> RemoveResult(TaskId id) = 0;
    virtual void Stop() {};

    virtual ~Manager() {
        Stop();
    };
};