#pragma once 
#include <queue>
#include <shared_mutex>
#include <optional>

template<typename T>
class ThreadSafeQueue {
public:
    using ValueType = T;
    using ConstReference = const T&;

    void push(ConstReference task) {
        emplace(task);
    }

    template<typename... Args>
    void emplace(Args&&... args) {
        std::lock_guard lock(mutex_);
        queue_.emplace(std::forward<Args>(args)...);
    }

    std::optional<ValueType> pop() {
        std::lock_guard lock(mutex_);
        if(queue_.empty())
            return std::nullopt;

        auto value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    bool empty() const {
        std::shared_lock lock(mutex_);
        return queue_.empty();
    }

private:
    std::queue<ValueType> queue_;
    mutable std::shared_mutex mutex_;
};