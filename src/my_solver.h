#pragma once

#include "solver.h"
#include "common.h"

#include <math.h>


class EquationSolver : public Solver<Task, Result> {
public:
    Result Solve(const Task& task) override {
        const auto& [a, b, c] = task;

        const auto D = b * b - 4. * a * c;

        if(D < 0)
            return NoSolution{};

        if(IsZero(a)) {
            if(IsZero(b)) {
                if(IsZero(c)) { // a == b == c == 0
                    return InfiniteNumberOfRoots{};
                }
                return NoSolution{};
            }
            return OneRootSolution{c / b};
        }

        if(D > 0.) 
            return TwoRootSolution{(-b - sqrt(D))/ (2. * a), (-b + sqrt(D))/ (2. * a)};
        
        return OneRootSolution{-b / (2. * a)};
    }

private:
    bool IsZero(Task::const_reference value) {
        return std::abs(value) < std::numeric_limits<Task::value_type>::epsilon();
    }
};