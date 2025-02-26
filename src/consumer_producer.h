#pragma once

#include <queue>
#include <condition_variable>
#include <unordered_map>
#include <optional>
#include <shared_mutex>
#include <functional>
#include <sstream>


template<typename T>
class TaskQueueImpl {
public:
    using ValueType = T;
    using ConstReference = const T&;

    void AddTask(ConstReference task) {
        Emplace(task);
    }

    template<typename... Args>
    void Emplace(Args&&... args) {
        {
            std::lock_guard lock(mutex_);
            queue_.emplace(std::forward<Args>(args)...);
        }
        cv_.notify_one();
    }

    std::optional<ValueType> PopTask() {
        std::unique_lock lock(mutex_);
        
        cv_.wait(lock, [this] { 
            return stopped_ || !queue_.empty();
        });

        if(stopped_ && queue_.empty())
            return std::nullopt;

        auto value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    void Stop()  noexcept {
        {
            std::unique_lock lock(mutex_);
            stopped_ = true;
        }
        cv_.notify_all();
    }

private:
    std::queue<ValueType> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_{false};
};

template<typename Key, typename Value>
class ResultCacheImpl {
public:
    using KeyType = Key;
    using KeyConstRef = const KeyType&;
    using ValueType = Value;
    using ValueConstRef = const ValueType&;

    std::optional<std::reference_wrapper<const ValueType>> Get(KeyConstRef key) const {
        std::shared_lock lock(mutex_);
        if(const auto it = map_.find(key); it != map_.cend()) {
            return  std::cref(it->second);
        }

        return std::nullopt;
    }

    void Add(KeyConstRef key, ValueConstRef value) {
        Emplace(key, value);
    }

    template<typename... Args>
    void Emplace(Args&&... args) {
        std::lock_guard lock(mutex_);
        map_.emplace(std::forward<Args>(args)...);
    }

    bool Contains(KeyConstRef key) const {
        std::shared_lock lock(mutex_);
        return map_.count(key) != 0u;
    }

private:
    std::unordered_map<ValueType, KeyType> map_;
    mutable std::shared_mutex mutex_;
};

class Logger {
public:
    virtual ~Logger() = default;

    void Log(const std::string& text) {
        std::lock_guard lock(mutex_);
        DoLog(text);
    }

    template<typename T>
    Logger& operator<<(const T& value) {
        {
            std::lock_guard lock(mutex_);
            std::ostringstream oss;
            oss << value;
            DoLog(oss.str());
        }
        return *this;
    }

    using StreamManipulator = std::ostream& (*)(std::ostream&);
    Logger& operator<<(StreamManipulator manip) {
        {
            std::lock_guard lock(mutex_);
            std::ostringstream oss;
            oss << manip;
            DoLog(oss.str());
        }
        return *this;
    }

protected:
    virtual void DoLog(const std::string& text) = 0;

private:
    std::mutex mutex_;
};

class ConsoleLogger : public Logger {
protected:
    void DoLog(const std::string& text) override {
        std::clog << text;
    }
};

class Server {
public:
    // server very long operation
    static int Compute(int value) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return value * value;
    }
};

using TaskId = int;

template<typename Func>
struct TaskImpl {
    TaskImpl(TaskId id, Func f)
        : task_id_(id), func_(std::move(f)) {}

    template<typename... Args>
    std::invoke_result_t<Func> operator()(Args&&... args) const {
        return std::invoke(func_, std::forward<Args>(args)...);
    }

    TaskId GetId() const {
        return task_id_;
    }

private:
    TaskId task_id_;
    Func func_;
};

using TaskFunction = std::function<int()>;
using Task = TaskImpl<TaskFunction>;
using TaskReturnValue = std::invoke_result_t<TaskFunction>;

using TaskQueue = TaskQueueImpl<Task>;
using ResultCache = ResultCacheImpl<TaskId, TaskReturnValue>;

class Consumer {
public:
    Consumer(TaskQueue& task_queue, ResultCache& result_cache, int id, Logger& logger) 
        : task_queue_(task_queue)
        , result_cache_(result_cache)
        , consumer_id_(id)
        , logger_(logger) { }

    void Consume() {
        while(true) {
            const auto task = task_queue_.PopTask();
            if(!task) {
                logger_ << "Consumer " << consumer_id_ 
                    << " EXITING!" <<  std::endl;
                return;

            }
            
            if(result_cache_.Contains(task->GetId())) {
                logger_ << "Consumer " << consumer_id_ 
                << " get cached result " << *result_cache_.Get(task->GetId())
                << " of task " << task->GetId() << std::endl;

                continue;
            }

            result_cache_.Emplace(task->GetId(), (*task)());

            logger_ << "Consumer " << consumer_id_ 
                << " save result " << *result_cache_.Get(task->GetId())
                << " of task " << task->GetId() << std::endl;
        }
    }
private:
    TaskQueue& task_queue_;
    ResultCache& result_cache_;
    int consumer_id_;
    Logger& logger_;
};

class Producer {
public:
    Producer(TaskQueue& task_queue, ResultCache& result_cache, int id, size_t task_count, Logger& logger) 
        : task_queue_(task_queue)
        , result_cache_(result_cache)
        , producer_id_(id)
        , task_count_(task_count)
        , logger_(logger) { }

    void AddTask(Task task) {
        task_queue_.Emplace(std::move(task));
    }

    void Produce() {
        for(size_t i = 0; i < task_count_; ++i) {
            const auto taskId = producer_id_ * task_count_ + i;
            if(result_cache_.Contains(taskId))
                continue;

            task_queue_.Emplace(taskId, [this, taskId] {
                return Server::Compute(taskId);
            });

            logger_ << "Producer " << producer_id_ << " created task: " << taskId << std::endl;
        
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

private:
    TaskQueue& task_queue_;
    ResultCache& result_cache_;
    int producer_id_;
    size_t task_count_;
    Logger& logger_;
};

class ThreadPool {
public:
    ThreadPool(unsigned int num_producers, 
        unsigned int num_consumers, size_t tasks_per_producer, 
        Logger& logger)
        : logger_(logger) {
        producers_.reserve(num_producers);
        producers_threads_.reserve(num_producers);
        for(unsigned int i = 0u; i < num_producers; ++i) {
            auto& producer = producers_.emplace_back(task_queue_, result_cache_, i, tasks_per_producer, logger_);
            producers_threads_.emplace_back(&Producer::Produce, &producer);
        }

        consumers_.reserve(num_consumers);
        consumers_threads_.reserve(num_consumers);
        for(unsigned int i = 0u; i < num_consumers; ++i) {
            auto& consumer = consumers_.emplace_back(task_queue_, result_cache_, i, logger_);
            consumers_threads_.emplace_back(&Consumer::Consume, &consumer);
        }
    }

    std::optional<std::reference_wrapper<const TaskReturnValue>> GetResult(TaskId id) {
        return result_cache_.Get(id);
    }

    void Stop() {
        if(!stopped_) {
            stopped_ = true;
            task_queue_.Stop();
            for(auto& thread : producers_threads_)
                thread.join();

            for(auto& thread : consumers_threads_)
                thread.join();
        }
    }

    ~ThreadPool() {
        Stop();
    }

private:
    TaskQueue task_queue_;
    ResultCache result_cache_;
    Logger& logger_;
    std::vector<Producer> producers_;
    std::vector<Consumer> consumers_;
    std::vector<std::thread> producers_threads_;
    std::vector<std::thread> consumers_threads_;
    bool stopped_{false};
};