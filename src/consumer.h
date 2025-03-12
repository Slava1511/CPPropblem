#pragma once

#include "common.h"
#include "manager.h"

template<typename TaskId, typename TFunc>
class Consumer {
    using TaskManager = Manager<TaskId, TFunc>;
    using Task = typename TaskManager::Task;
public:
    Consumer(TaskManager& manager, int id, Logger& logger) 
        : manager_(manager)
        , consumer_id_(id)
        , logger_(logger) { }

    void Consume() {
        while( running_ || manager_.HasTask()) {
            const auto task = manager_.GetTask();
            if(!task) {
                little_sleep(SLEEP_TIME);
                continue;
            }
            logger_ << "Consumer " << consumer_id_ 
                << " Get task " << task->GetId() << std::endl;
                
            const auto res = manager_.CalculateTask(*task);
            manager_.SaveResult(task->GetId(), res);

            logger_ << "Consumer " << consumer_id_ 
                << " save result " << res
                << " of task " << task->GetId() << std::endl;
        };
        logger_ << "Consumer " << consumer_id_ 
                    << " exiting! " << std::endl;
    }

    void Stop() {
        running_ = false;
    }
private:
    TaskManager& manager_;
    int consumer_id_;
    Logger& logger_;
    bool running_{true};
};