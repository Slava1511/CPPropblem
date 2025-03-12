#pragma once

#include "producer.h"
#include "consumer.h"
#include "thread_safe_map.h"
#include "thread_safe_queue.h"

using TaskFunction = std::function<int()>;
using TaskId = int;

class MyManager : public Manager<TaskId, TaskFunction> {
private:
    using TaskQueue = ThreadSafeQueue<Task>;
    using ResultCache = ThreadSafeMap<TaskId, TaskReturnValue>;
    using Producer = Producer<TaskId, TaskFunction>;
    using Consumer = Consumer<TaskId, TaskFunction>;
public:
    MyManager(unsigned int num_producers, 
        unsigned int num_consumers, size_t tasks_per_producer, 
        Logger& logger) 
        : logger_(logger)
        {
            producers_threads_.reserve(num_producers);
            for(unsigned int i = 0u; i < num_producers; ++i) {
                auto& producer = producers_.emplace_back(*this, i, tasks_per_producer, logger_);
                producers_threads_.emplace_back(&Producer::Produce, &producer);
            }
    
            consumers_threads_.reserve(num_consumers);
            for(unsigned int i = 0u; i < num_consumers; ++i) {
                auto& consumer = consumers_.emplace_back(*this, i, logger_);
                consumers_threads_.emplace_back(&Consumer::Consume, &consumer);
            }
        }


    bool AddTask(const Task& task) override {
        task_queue_.push(task);
        return true;
    }
    std::optional<Task> GetTask() override {
        return task_queue_.pop();
    }
    bool HasTask() override {
        return !task_queue_.empty();
    }
    TaskReturnValue CalculateTask(const Task& task) override {
        return task();
    }
    void SaveResult(TaskId id, const TaskReturnValue& value) override {
        result_cache_.Add(id, value);
    }
    std::optional<TaskReturnValue> RemoveResult(TaskId id) override {
        return result_cache_.Get(id);
    }

    void Stop() override {
        if(!stopped_) {
            stopped_ = true;
            for(auto& p : producers_)
                p.Stop();
            for(auto& c : consumers_)
                c.Stop();

            for(auto& thread : producers_threads_)
                thread.join();

            for(auto& thread : consumers_threads_)
                thread.join();
        }
    }

private:
    TaskQueue task_queue_;
    ResultCache result_cache_;
    Logger& logger_;

    std::deque<Producer> producers_;
    std::deque<Consumer> consumers_;
    std::vector<std::thread> producers_threads_;
    std::vector<std::thread> consumers_threads_;
    bool stopped_{false};
};