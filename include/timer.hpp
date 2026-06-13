#pragma once

#include <chrono>

class Timer {
public:
    Timer()
        : running_(false),
          elapsed_(0.0)
    {}

    void start()
    {
        running_ = true;
        start_time_ = clock_type::now();
    }

    void stop()
    {
        if (running_) {
            auto end_time = clock_type::now();

            elapsed_ +=
                std::chrono::duration<double>(
                    end_time - start_time_
                ).count();

            running_ = false;
        }
    }

    void reset()
    {
        running_ = false;
        elapsed_ = 0.0;
    }

    double seconds() const
    {
        return elapsed_;
    }

private:
    using clock_type = std::chrono::high_resolution_clock;

    bool running_;
    double elapsed_;
    clock_type::time_point start_time_;
};