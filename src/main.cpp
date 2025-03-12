#include <iostream>
#include "logger.h"
#include "my_manager.h"

class ConsoleLogger : public Logger {
    protected:
        void DoLog(const std::string& text) override {
            std::clog << text;
        }
    };


int main() {
    static constexpr unsigned num_producers = 3;
    static constexpr unsigned num_consumers = 2;
    static constexpr size_t tasks_per_producer = 5;
    static ConsoleLogger logger;

    MyManager manager(num_producers, 
        num_consumers, tasks_per_producer, logger);

    std::this_thread::sleep_for(std::chrono::seconds(5)); // Giving time for consumers and producers
    manager.Stop();
}



// int main() {
//     static constexpr unsigned num_producers = 3;
//     static constexpr unsigned num_consumers = 2;
//     static constexpr size_t tasks_per_producer = 5;
//     static ConsoleLogger logger;

//     ThreadPool pool(num_producers, num_consumers, tasks_per_producer, logger);

//     std::this_thread::sleep_for(std::chrono::seconds(5)); // Giving time for consumers and producers
//     pool.Stop();

//     // example of getting results
//     for (unsigned i = 0; i < num_producers; ++i) {
//         for (size_t j = 0; j < tasks_per_producer; ++j) {
//             int taskId = i * tasks_per_producer + j; // Уникальный идентификатор задачи
//             const auto result = pool.GetResult(taskId);
//             if (result) {
//                 logger << "Retrieved cached result for task " 
//                           << taskId << ": " << *result << std::endl;
//             } else {
//                 logger << "No cached result for task " << taskId << std::endl;
//             }
//         }
//     }
// }