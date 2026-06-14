#include "solver_mpi.hpp"
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <omp.h>

SolverMPI::SolverMPI(const MPIDomain& domain, int steps, double u, double v, double dt, double kappa)
    : domain_(domain), local_nx_(domain.local_nx()), ny_(domain.local_ny()), steps_(steps),
      u_(u), v_(v), dt_(dt), kappa_(kappa), dx_(1.0 / domain.global_nx()), dy_(1.0 / domain.global_ny()),
      T_((local_nx_ + 2) * ny_, 0.0), T_new_((local_nx_ + 2) * ny_, 0.0)
{
    if (local_nx_ < 1 || ny_ <= 2) {
        throw std::runtime_error("Invalid local dimensions allocation.");
    }
}

int SolverMPI::index(int i, int j) const {
    return i * ny_ + j;
}

void SolverMPI::initialize() {
    int start_x = domain_.start_x();
    int g_nx = domain_.global_nx();
    int g_ny = domain_.global_ny();

    #pragma omp parallel for collapse(2)
    for (int i = 1; i <= local_nx_; ++i) {
        for (int j = 0; j < ny_; ++j) {
            double x = static_cast<double>(start_x + (i - 1)) / g_nx;
            double y = static_cast<double>(j) / g_ny;
            double r2 = (x - 0.5)*(x - 0.5) + (y - 0.5)*(y - 0.5);
            T_[index(i, j)] = std::exp(-r2 / (2.0 * 0.05 * 0.05));
        }
    }
    exchange_halos();
}

void SolverMPI::exchange_halos() {
    comm_timer_.start();

    MPI_Sendrecv(
        &T_[index(1, 0)], ny_, MPI_DOUBLE, domain_.left_rank(), 0,
        &T_[index(local_nx_ + 1, 0)], ny_, MPI_DOUBLE, domain_.right_rank(), 0,
        domain_.comm(), MPI_STATUS_IGNORE
    );

    MPI_Sendrecv(
        &T_[index(local_nx_, 0)], ny_, MPI_DOUBLE, domain_.right_rank(), 1,
        &T_[index(0, 0)], ny_, MPI_DOUBLE, domain_.left_rank(), 1,
        domain_.comm(), MPI_STATUS_IGNORE
    );

    comm_timer_.stop();
}

void SolverMPI::step() {
    exchange_halos();
    double dx2 = dx_ * dx_;
    double dy2 = dy_ * dy_;

    #pragma omp parallel for collapse(2)
    for (int i = 1; i <= local_nx_; ++i) {
        for (int j = 0; j < ny_; ++j) {
            int jp = (j == ny_ - 1) ? 0 : j + 1;
            int jm = (j == 0) ? ny_ - 1 : j - 1;

            double center = T_[index(i, j)];
            double laplacian = (T_[index(i+1, j)] - 2.0 * center + T_[index(i-1, j)]) / dx2 +
                               (T_[index(i, jp)] - 2.0 * center + T_[index(i, jm)]) / dy2;

            double dTdx = (T_[index(i+1, j)] - T_[index(i-1, j)]) / (2.0 * dx_);
            double dTdy = (T_[index(i, jp)] - T_[index(i, jm)]) / (2.0 * dy_);

            T_new_[index(i, j)] = center - dt_ * (u_ * dTdx + v_ * dTdy) + dt_ * kappa_ * laplacian;
        }
    }
    T_.swap(T_new_);
}


void SolverMPI::run() {
    compute_timer_.reset();
    comm_timer_.reset();

    compute_timer_.start();

    for (int n = 0; n < steps_; ++n) {
        step();
    }

    compute_timer_.stop();
}

double SolverMPI::compute_time() const { return compute_timer_.seconds(); }
double SolverMPI::comm_time() const {return comm_timer_.seconds();
}

vvoid SolverMPI::write_field(const std::string& filename) const {
    std::string rank_filename =
        filename + "_rank" + std::to_string(domain_.rank()) + ".dat";

    std::ofstream file(rank_filename);

    if (!file) {
        throw std::runtime_error("Could not open output file: " + rank_filename);
    }

    for (int i = 1; i <= local_nx_; ++i) {
        for (int j = 0; j < ny_; ++j) {
            double x =
                static_cast<double>(domain_.start_x() + (i - 1))
                / domain_.global_nx();

            double y =
                static_cast<double>(j)
                / domain_.global_ny();

            file << x << " "
                 << y << " "
                 << T_[index(i, j)] << "\n";
        }
        file << "\n";
    }
}