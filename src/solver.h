#pragma once

template<typename Task_, typename Result_>
class Solver {
public:
    using Task = Task_;
    using Result = Result_;

    virtual Result Solve(const Task& task) = 0;
protected:
    ~Solver() = default;
};