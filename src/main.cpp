#include <iostream>
#include <queue>
#include <array>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>
#include <syncstream>

static constexpr size_t CONSUMERS = 3;
static constexpr size_t PRODUCERS = 5;

// task queue
std::queue<int> taskQueue;
std::mutex queueMutex;
std::condition_variable queueCV;

// cache
std::unordered_map<int, int> resultCache;
std::mutex cacheMutex;

std::mt19937 randomGenerator;
std::random_device device;

std::atomic_bool running(true);

// server very long operation
int serverCompute(int value) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return value * value;
}

void producer(int id) {
    while (running)
    {
        {
            std::lock_guard lock(queueMutex);
            std::uniform_int_distribution<int> range(0, PRODUCERS / 2u);
            const auto value = range(randomGenerator);
            taskQueue.push(value);
            std::osyncstream(std::cout) << "Produser " << id << " added task: " << value << std::endl;
        }
        queueCV.notify_one();
        // just a little pause between tasks
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
    
void consumer(int id) {
    while (running) {
        int task;
        {
            std::unique_lock lock(queueMutex);
            queueCV.wait(lock, [id] { 
                std::osyncstream(std::cout) << "Consumer " << id << " awaiting for task" << std::endl;
                return !taskQueue.empty() || ! running; 
            });

            if(!running && taskQueue.empty()) 
                break;

            task = taskQueue.front();
            taskQueue.pop();
        }

        std::osyncstream(std::cout) << "Consumer " << id << " got task: " << task << std::endl;

        // check cache
        {
            std::lock_guard lock(cacheMutex);
            if(const auto it = resultCache.find(task); it != resultCache.cend()) {
                std::osyncstream(std::cout) << "Consumer " << id << " got cached result: " << it->second 
                    << " for task " << task << std::endl;
                continue;
            }
        }
        
        // server request
        const auto result = serverCompute(task);
        
        // save into cache
        {
            std::lock_guard lock(cacheMutex);
            resultCache[task] = result;
        }

        std::osyncstream(std::cout) << "Consumer " << id << " computed result: " << result 
                    << " for task " << task << std::endl;
    }
}

int main() {
    randomGenerator.seed(device());

    std::vector<std::thread> consumers;
    consumers.reserve(CONSUMERS);
    for(size_t i = 0u; i < CONSUMERS; ++i)
        consumers.emplace_back(consumer, (int)(i + 1));

    std::vector<std::thread> producers;
    producers.reserve(PRODUCERS);
    for(size_t i = 0u; i < PRODUCERS; ++i)
        producers.emplace_back(producer, (int)(i + 1));

    // give time for consumers to deal with tasks 
    std::this_thread::sleep_for(std::chrono::seconds(10));
    running = false;

    queueCV.notify_all();

    for (auto& p : producers)
        p.join();
    for (auto& c : consumers)
        c.join();
}