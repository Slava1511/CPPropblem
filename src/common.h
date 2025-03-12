#pragma once

#include "thread_safe_queue.h"
#include "thread_safe_map.h"
#include "logger.h"
#include "task.h"

#include <thread>
#include <chrono>

class Server {
public:
    // server very long operation
    static int Compute(int value) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return value * value;
    }
};


static constexpr auto SLEEP_TIME = std::chrono::microseconds(15);

static void little_sleep(std::chrono::microseconds ms) {
    const auto start = std::chrono::high_resolution_clock::now();
    const auto end = start + ms;
    do {
        std::this_thread::yield();
    } while (std::chrono::high_resolution_clock::now() < end);
}