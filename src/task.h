#pragma once
#include <functional>

template<typename T, typename Func>
struct TaskImpl {
    using TaskId = T;
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

