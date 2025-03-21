#pragma once

#include "logger.h"

template<typename Task_, typename Ticket_, typename Result_>
class Storage {
public:
    using Task = Task_;
    using Ticket = Ticket_;
    using Result = Result_;

public:
    virtual Ticket AddTask(const Task& task) = 0;
    virtual std::pair<Ticket, Task> GetTask() = 0;
    virtual bool HasTask() = 0;
    virtual void SaveResult(const Ticket& ticket, const Result& value) = 0;
    virtual Result GetResult(const Ticket& ticket) = 0;

protected:
    ~Storage() = default;
};