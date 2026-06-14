#pragma once
#include "solver.hpp"
#include "mpi_domain.hpp"
#include "timer.hpp"
#include <vector>

class SolverMPI : public Solver {
public:
    SolverMPI(const MPIDomain& domain, int steps, double u, double v, double dt, double kappa);
    
    void initialize() override;
    void run() override;
    void write_field(const std::string& filename) const override;
    double compute_time() const override;
    double comm_time() const;

private:
    const MPIDomain& domain_;
    int local_nx_, ny_, steps_;
    double u_, v_;
    double dt_, kappa_;
    double dx_, dy_;

    std::vector<double> T_;
    std::vector<double> T_new_;

    int index(int i, int j) const;
    void exchange_halos();
    void step();
    Timer compute_timer_;
    Timer comm_timer_;
};