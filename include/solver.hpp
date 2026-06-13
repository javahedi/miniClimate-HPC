#pragma once 

#include <vector>
#include <string>


/*
    Solver for simplae 2d advection-diffusion equaiton:
        dT/dt + u dT/dx + v dT/dy = kappa * Laplacian(T)

    This is a standard stencil problem.
    It is not meant to be a full climate model.
    It is a compact proxy for climate/HPC kernels.

*/


class Solver {
    public:
        Solver(int nx, int ny, int steps, double dt, double kappa);

        void initialize();
        void run();
        void write_field(const std::string& filename) const;

    private:
    int nx_, ny_, steps_;
    double dt_, kappa_;
    double dx_, dy_;

    std::vector<double> T_;      // current temperature field
    std::vector<double> T_new_;  // updated field


    int index(int i, int j) const;
    void step();
    
 };