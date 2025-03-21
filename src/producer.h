#pragma once

#include "common.h"
#include "storage.h"
#include <vector>
#include <thread>

#include <random>

// helper type for the visitor
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

class GetRandomNumber {
public:
    GetRandomNumber(double from, double to) : from_(from), to_(to) {
        std::random_device device;
        random_generator_.seed(device());
    }

    double Get() {
        return Get(from_, to_);
    }

    double Get(double from, double to) {
        std::uniform_int_distribution<int> range(from, to);
        return range(random_generator_);
    }

private:
    double from_;
    double to_;
    std::mt19937 random_generator_;
};

template<typename Task_, typename Ticket_, typename Result_>
class Producer {
    using Storage = Storage<Task_, Ticket_, Result_>;
    using Task = typename Storage::Task;
    using Ticket = typename Storage::Ticket;
    using Result = typename Storage::Result;
public:
    Producer(Storage& storage, int id, size_t task_count, Logger& logger) 
        : storage_(storage)
        , producer_id_(id)
        , task_count_(task_count)
        , logger_(logger) { }

    void Produce() {
        for(size_t i = 0; i < task_count_; ++i) {
            if(!running_)
                break;

            GetRandomNumber rn(-100., 100.);
            const auto ticket = storage_.AddTask({rn.Get(), rn.Get(), rn.Get()});
            logger_ << "Producer " << producer_id_ << " created task: " << ticket << std::endl;

            const auto visitor = overloaded {
                [this](const NoSolution&){ logger_ << "No solution\n"; },
                [this](const OneRootSolution& s){ logger_ << "x = " << s.root_ << std::endl; },
                [this](const TwoRootSolution& s){ 
                    logger_ << "x1 = " << s.root1_ 
                    << "\n x2 = " << s.root2_ << std::endl;},
                [this](const InfiniteNumberOfRoots&){ logger_ << "Inf number of roots\n"; },
            };

            const auto res = storage_.GetResult(ticket);
            logger_ << "Producer " << producer_id_ 
            << " Get result of task " << ticket << std::endl;
            std::visit( visitor, res);
        }
        logger_ << "Producer " << producer_id_ 
                    << " exiting! " << std::endl;
    }

    void Stop() {
        if(running_) {
            running_ = false;
        }
        
    }

    ~Producer() {
        Stop();
    }

private:
    Storage& storage_;
    int producer_id_;
    size_t task_count_;
    Logger& logger_;
    bool running_{true};
};