#pragma once

#include "thread_safe_queue.h"
#include "thread_safe_map.h"
#include "logger.h"

#include <array>
#include <variant>

struct NoSolution{};
struct OneRootSolution {
    double root_{ 0. };
};
struct TwoRootSolution {
    double root1_{ 0. };
    double root2_{ 0. };
};
struct InfiniteNumberOfRoots { };

using Task = std::array<double, 3u>;
using Ticket = int;
using Result = std::variant<NoSolution, OneRootSolution, TwoRootSolution, InfiniteNumberOfRoots>;