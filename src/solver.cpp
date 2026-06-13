#include "solver.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <omp.h> 

Solver::Solver(int nx, int ny, int steps, double u, double v, double dt, double kappa)
    : nx_(nx), 
      ny_(ny), 
      steps_(steps), 
      u_(u),
      v_(v),
      dt_(dt), 
      kappa_(kappa),
      dx_(1.0 / nx),
      dy_(1.0 / ny),
      T_(nx * ny, 0.0),
      T_new_(nx * ny, 0.0)
{
    if (nx_ <= 2 || ny_ <= 2) {
        throw std::runtime_error("Grid size must be larger than 2 in both directions.");
    }
}

int Solver::index(int i, int j) const {
    return i * ny_ + j;
}

void Solver::initialize() {
    for (int i = 0; i < nx_; ++i) {
        for (int j = 0; j < ny_; ++j) {
            double x = static_cast<double>(i) / nx_;
            double y = static_cast<double>(j) / ny_;

            double cx = 0.5;
            double cy = 0.5;
            double sigma = 0.05;

            double r2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
            T_[index(i, j)] = std::exp(-r2 / (2.0 * sigma * sigma));
        }
    }
}

// This function is now called by ALL threads simultaneously inside the parallel region
void Solver::step() {
    double dx2 = dx_ * dx_;
    double dy2 = dy_ * dy_;

    // Notice there is NO `parallel` keyword here, only `omp for`.
    // It hitches a ride on the thread pool already spawned in run().
    #pragma omp for schedule(static)
    for (int i = 0; i < nx_; ++i) {
        int ip = (i + 1) % nx_;
        int im = (i - 1 + nx_) % nx_; 
        
        for (int j = 0; j < ny_; ++j) {
            int jp = (j + 1) % ny_;
            int jm = (j - 1 + ny_) % ny_; 

            double center = T_[index(i, j)];

            double laplacian =
                (T_[index(ip, j)] - 2.0 * center + T_[index(im, j)]) / dx2 +
                (T_[index(i, jp)] - 2.0 * center + T_[index(i, jm)]) / dy2;

            double dTdx = (T_[index(ip, j)] - T_[index(im, j)]) / (2.0 * dx_);
            double dTdy = (T_[index(i, jp)] - T_[index(i, jm)]) / (2.0 * dy_);

            T_new_[index(i, j)] = center - dt_ * (u_ * dTdx + v_ * dTdy) + dt_ * kappa_ * laplacian;
        }
    } // <-- There is an implicit OpenMP barrier here. All threads wait until the full grid step is computed.
}

void Solver::run()
{
    compute_timer_.reset();
    compute_timer_.start();

    // Spawn the OpenMP thread pool EXACTLY ONCE here
    #pragma omp parallel
    {
        for (int n = 0; n < steps_; ++n) {
            
            // All threads jump into step() together and split up the rows
            step(); 

            // Only ONE thread swaps the pointers while others wait
            #pragma omp single
            {
                T_.swap(T_new_);
            } // <-- Implicit barrier here ensures the swap finishes before anyone starts the next time step loop
        }
    }

    compute_timer_.stop();
}

double Solver::compute_time() const {
    return compute_timer_.seconds();
}

void Solver::write_field(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("Could not open output file");
    }
    for (int i = 0; i < nx_; ++i) {
        for (int j = 0; j < ny_; ++j) {
            double x = static_cast<double>(i) / nx_;
            double y = static_cast<double>(j) / ny_;
            file << x << " " << y << " " << T_[index(i, j)] << "\n";
        }
        file << "\n";
    }
    std::cout << "Wrote field to " << filename << "\n";
}