#pragma once

#include "common.h"

#include "storage.h"
#include "solver.h"

template<typename Task_, typename Ticket_, typename Result_>
class Consumer {
    using Storage = Storage<Task_, Ticket_, Result_>;
    using Solver = Solver<Task_, Result_>;
    using Task = typename Storage::Task;
    using Ticket = typename Storage::Ticket;
    using Result = typename Storage::Result;
public:
    Consumer(Storage& storage, Solver& solver, int id, Logger& logger) 
        : storage_(storage)
        , solver_(solver)
        , consumer_id_(id)
        , logger_(logger) { }

    void Consume() {
        while( running_ || storage_.HasTask()) {
            const auto [ticket, task] = storage_.GetTask();

            logger_ << "Consumer " << consumer_id_ 
                << " Get task " << ticket << " " 
                << task.at(0) << ", " << task.at(1) << ", " << task.at(2) << std::endl;
                
            const auto res = solver_.Solve(task);
            storage_.SaveResult(ticket, res);

            logger_ << "Consumer " << consumer_id_ 
                << " save result of task " << ticket << std::endl;
        };
        logger_ << "Consumer " << consumer_id_ 
                    << " exiting! " << std::endl;
    }

    void Stop() {
        running_ = false;
    }
private:
    Storage& storage_;
    Solver& solver_;
    int consumer_id_;
    Logger& logger_;
    bool running_{true};
};