#pragma once 

#include <string>

/*
    Abstract Base Class for miniClimate Equation Solvers.
    Enforces a strict lifecycle pattern across different parallel models.
*/
class Solver {
public:
    virtual ~Solver() = default;

    // Core execution lifecycle hooks
    virtual void initialize() = 0;
    virtual void run() = 0;
    virtual void write_field(const std::string& filename) const = 0;
    
    // Performance benchmarking accessors
    virtual double compute_time() const = 0;
};