#include <iostream>
#include "logger.h"
#include "my_storage.h"
#include "my_solver.h"
#include "producer.h"
#include "consumer.h"

class ConsoleLogger : public Logger {
    protected:
        void DoLog(const std::string& text) override {
            std::clog << text;
        }
    };


int main() {
    static constexpr unsigned num_producers = 10;
    static constexpr unsigned num_consumers = 4;
    static constexpr size_t tasks_per_producer = 10;
    static ConsoleLogger logger;

    using Producer = Producer<Task, Ticket, Result>;
    using Consumer = Consumer<Task, Ticket, Result>;
    using Storage = MyStorage;

    Storage storage;
    EquationSolver solver;

    std::deque<Producer> producers_;
    std::deque<Consumer> consumers_;
    std::vector<std::thread> producers_threads_;
    std::vector<std::thread> consumers_threads_;

    producers_threads_.reserve(num_producers);
    for(unsigned int i = 0u; i < num_producers; ++i) {
        auto& producer = producers_.emplace_back(storage, i, tasks_per_producer, logger);
        producers_threads_.emplace_back(&Producer::Produce, &producer);
    }

    consumers_threads_.reserve(num_consumers);
    for(unsigned int i = 0u; i < num_consumers; ++i) {
        auto& consumer = consumers_.emplace_back(storage, solver, i, logger);
        consumers_threads_.emplace_back(&Consumer::Consume, &consumer);
    }


    std::this_thread::sleep_for(std::chrono::seconds(5)); // Giving time for consumers and producers

    for(auto& p : producers_)
        p.Stop();
    for(auto& c : consumers_)
        c.Stop();

    for(auto& thread : producers_threads_)
        thread.join();

    for(auto& thread : consumers_threads_)
        thread.join();
}