#include "solver_openmp.hpp"
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <omp.h>

SolverOpenMP::SolverOpenMP(int nx, int ny, int steps, double u, double v, double dt, double kappa)
    : nx_(nx), ny_(ny), steps_(steps), u_(u), v_(v), dt_(dt), kappa_(kappa),
      dx_(1.0 / nx), dy_(1.0 / ny), T_(nx * ny, 0.0), T_new_(nx * ny, 0.0)
{
    if (nx_ <= 2 || ny_ <= 2) {
        throw std::runtime_error("Grid size must be larger than 2.");
    }
}

int SolverOpenMP::index(int i, int j) const {
    // Modulo logic wraps around local coordinate bounds safely
    int wrapped_i = (i + nx_) % nx_;
    int wrapped_j = (j + ny_) % ny_;
    return wrapped_i * ny_ + wrapped_j;
}

void SolverOpenMP::initialize() {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < nx_; ++i) {
        for (int j = 0; j < ny_; ++j) {
            double x = static_cast<double>(i) / nx_;
            double y = static_cast<double>(j) / ny_;
            double r2 = (x - 0.5)*(x - 0.5) + (y - 0.5)*(y - 0.5);
            T_[index(i, j)] = std::exp(-r2 / (2.0 * 0.05 * 0.05));
        }
    }
}

void SolverOpenMP::step() {
    double dx2 = dx_ * dx_;
    double dy2 = dy_ * dy_;

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < nx_; ++i) {
        for (int j = 0; j < ny_; ++j) {
            double center = T_[index(i, j)];
            double laplacian = (T_[index(i+1, j)] - 2.0 * center + T_[index(i-1, j)]) / dx2 +
                               (T_[index(i, j+1)] - 2.0 * center + T_[index(i, j-1)]) / dy2;

            double dTdx = (T_[index(i+1, j)] - T_[index(i-1, j)]) / (2.0 * dx_);
            double dTdy = (T_[index(i, j+1)] - T_[index(i, j-1)]) / (2.0 * dy_);

            T_new_[index(i, j)] = center - dt_ * (u_ * dTdx + v_ * dTdy) + dt_ * kappa_ * laplacian;
        }
    }
    T_.swap(T_new_);
}

void SolverOpenMP::run() {
    compute_timer_.reset(); compute_timer_.start();
    for (int n = 0; n < steps_; ++n) { step(); }
    compute_timer_.stop();
}

double SolverOpenMP::compute_time() const { return compute_timer_.seconds(); }

void SolverOpenMP::write_field(const std::string& filename) const {
    std::ofstream file(filename);
    for (int i = 0; i < nx_; ++i) {
        for (int j = 0; j < ny_; ++j) {
            file << T_[index(i, j)] << (j == ny_ - 1 ? "" : " ");
        }
        file << "\n";
    }
}