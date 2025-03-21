#pragma once

#include "common.h"
#include "storage.h"
#include "thread_safe_map.h"
#include "thread_safe_queue.h"

#include <thread>
#include <chrono>

static constexpr auto SLEEP_TIME = std::chrono::microseconds(15);

static void little_sleep(std::chrono::microseconds ms) {
    const auto start = std::chrono::high_resolution_clock::now();
    const auto end = start + ms;
    do {
        std::this_thread::yield();
    } while (std::chrono::high_resolution_clock::now() < end);
}



class MyStorage : public Storage<Task, Ticket, Result> {
private:
    using TaskQueue = ThreadSafeQueue<std::pair<Ticket, Task>>;
    using ResultCache = ThreadSafeMap<Ticket, Result>;
public:
    MyStorage() = default;

    Ticket AddTask(const Task& task) override {
        const auto ticket = GetTicket();
        task_queue_.emplace(ticket, task);
        return ticket;
    }

    std::pair<Ticket, Task> GetTask() override {
        while(true) {
            const auto res = task_queue_.pop();
            if(res) {
                return *res;
            }

            little_sleep(SLEEP_TIME);
        }
    }

    bool HasTask() override {
        return !task_queue_.empty();
    }

    void SaveResult(const Ticket& ticket, const Result& value) override {
        result_cache_.Add(ticket, value);
    }

    Result GetResult(const Ticket& ticket) override {
        while(true) {
            const auto res = result_cache_.Get(ticket);

            if(res) {
                return *res;
            }

            little_sleep(SLEEP_TIME);
        }
    }

private:
    Ticket GetTicket() {
        std::unique_lock lock(mutex_);
        static Ticket uniqueTicket = 0;
        return uniqueTicket++;
    }

private:
    std::mutex mutex_;
    TaskQueue task_queue_;
    ResultCache result_cache_;
};