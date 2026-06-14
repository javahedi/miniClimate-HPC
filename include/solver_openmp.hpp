#pragma once 

#include "solver.hpp"
#include "timer.hpp"
#include <vector>



/*
    Solver for simplae 2d advection-diffusion equaiton:
        dT/dt + u dT/dx + v dT/dy = kappa * Laplacian(T)

    This is a standard stencil problem.
    It is not meant to be a full climate model.
    It is a compact proxy for climate/HPC kernels.

*/


class SolverOpenMp : public Solver {
    public:
        SolverOpenMp(int nx, int ny, int steps, double u, double v, double dt, double kappa);

        void initialize() override;
        void run() override;
        void write_field(const std::string& filename) const override;
        double compute_time() const override;

    private:
        int nx_, ny_, steps_;
        double u_, v_;
        double dt_, kappa_;
        double dx_, dy_;

        std::vector<double> T_;      // current temperature field
        std::vector<double> T_new_;  // updated field


        int index(int i, int j) const;
        void step();
        Timer compute_timer_;
    
 };