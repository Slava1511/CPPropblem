#pragma once

#include "common.h"
#include "manager.h"
#include <vector>
#include <thread>

template<typename TaskId, typename TFunc>
class Producer {
    using TaskManager = Manager<TaskId, TFunc>;
    using Task = typename TaskManager::Task;
public:
    Producer(TaskManager& manager, int id, size_t task_count, Logger& logger) 
        : manager_(manager)
        , producer_id_(id)
        , task_count_(task_count)
        , logger_(logger) { }

    void Produce() {
        for(size_t i = 0; i < task_count_; ++i) {
            if(!running_)
                break;

            const TaskId taskId = producer_id_ * task_count_ + i;
            
            manager_.AddTask(Task{taskId, [this, taskId] {
                return Server::Compute(taskId);
            }});

            logger_ << "Producer " << producer_id_ << " created task: " << taskId << std::endl;

            getResults_.emplace_back([this, taskId]{
                do {
                    const auto res = manager_.RemoveResult(taskId);
                    if(res) {
                        logger_ << "Producer " << producer_id_ 
                            << " Get result " << *res
                            << " of task " << taskId << std::endl;
    
                        break;
                    }
                    little_sleep(SLEEP_TIME);
                    
                } while(running_);
            });
        }
        logger_ << "Producer " << producer_id_ 
                    << " exiting! " << std::endl;
    }

    void Stop() {
        if(running_) {
            running_ = false;
            for(auto& t : getResults_)
                t.join();
        }
        
    }

    ~Producer() {
        Stop();
    }

private:
    TaskManager& manager_;
    int producer_id_;
    size_t task_count_;
    Logger& logger_;
    bool running_{true};

    std::vector<std::thread> getResults_;
};