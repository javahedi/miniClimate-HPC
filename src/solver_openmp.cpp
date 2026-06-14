#include "solver_openmp.hpp"
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <omp.h>

// Constructor refactored to use the local domain configurations
SolverOpenMP::SolverOpenMP(const MPIDomain& domain, int steps, double u, double v, double dt, double kappa)
    : domain_(domain),
      local_nx_(domain.local_nx()), 
      ny_(domain.local_ny()), 
      steps_(steps), 
      u_(u),
      v_(v),
      dt_(dt), 
      kappa_(kappa),
      dx_(1.0 / domain.global_nx()), // dx and dy must remain based on global scale for physics derivatives
      dy_(1.0 / domain.global_ny()),
      // Vector contains local_nx compute rows + 2 ghost/halo rows
      T_((local_nx_ + 2) * ny_, 0.0),
      T_new_((local_nx_ + 2) * ny_, 0.0)
{
    if (local_nx_ < 1 || ny_ <= 2) {
        throw std::runtime_error("Grid slice size calculation invalid.");
    }
}

int SolverOpenMP::index(int i, int j) const {
    // Modulo logic wraps around local coordinate bounds safely
    int wrapped_i = (i + nx_) % nx_;
    int wrapped_j = (j + ny_) % ny_;
    return wrapped_i * ny_ + wrapped_j;
}

void SolverOpenMP::initialize() {
    /*
       Each rank must compute its coordinates using its local domain offset: start_x_
       - Local compute row i goes from 1 to local_nx_
       - The actual global row is given by: start_x_ + (i - 1)
    */
    int global_nx = domain_.global_nx();
    int global_ny = domain_.global_ny();
    int start_x = domain_.start_x();

    double cx = 0.5;
    double cy = 0.5;
    double sigma = 0.05;

    #pragma omp parallel for collapse(2)
    for (int i = 1; i <= local_nx_; ++i) {
        for (int j = 0; j < ny_; ++j) {
            // Translate to global fraction for physics coordinate setting
            double x = static_cast<double>(start_x + (i - 1)) / global_nx;
            double y = static_cast<double>(j) / global_ny;
            double r2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
            T_[index(i, j)] = std::exp(-r2 / (2.0 * sigma * sigma));
        }
    }
    
    // Initial halo exchange to populate the ghost cells right away
    exchange_halos();
}

void Solver::exchange_halos() {
    /*
       Executes neighbor communications using standard synchronized MPI_Sendrecv.
       - Target rows are passed as contiguous pointer sequences along the inner 'j' arrays.
       - If is_periodic is false, MPI_PROC_NULL acts as a safe, silent no-op.
    */
    
    // 1. Send our top compute row (i=1) upward, receive our bottom ghost row (i=local_nx+1) from below
    MPI_Sendrecv(&T_[index(1, 0)], ny_, MPI_DOUBLE, domain_.left_rank(), 0,
                 &T_[index(local_nx_ + 1, 0)], ny_, MPI_DOUBLE, domain_.right_rank(), 0,
                 domain_.comm(), MPI_STATUS_IGNORE);

    // 2. Send our bottom compute row (i=local_nx) downward, receive our top ghost row (i=0) from above
    MPI_Sendrecv(&T_[index(local_nx_, 0)], ny_, MPI_DOUBLE, domain_.right_rank(), 1,
                 &T_[index(0, 0)], ny_, MPI_DOUBLE, domain_.left_rank(), 1,
                 domain_.comm(), MPI_STATUS_IGNORE);
}

void Solver::step() {
    // Synchronize halo boundaries across computing domains before math updates
    exchange_halos();

    double dx2 = dx_ * dx_;
    double dy2 = dy_ * dy_;

    // High performance hybrid math region
    #pragma omp parallel for collapse(2)
    for (int i = 1; i <= local_nx_; ++i) {
        for (int j = 0; j < ny_; ++j) {
            
            // Core coordinate stencils
            int ip = i + 1;
            int im = i - 1;
            
            // Handle periodic boundaries in the y-direction (fully local inside this rank)
            int jp = (j == ny_ - 1) ? 0 : j + 1;
            int jm = (j == 0) ? ny_ - 1 : j - 1;

            double center = T_[index(i, j)];

            // Numerical stencils for derivatives
            double laplacian = 
                (T_[index(ip, j)] - 2.0 * center + T_[index(im, j)]) / dx2 +
                (T_[index(i, jp)] - 2.0 * center + T_[index(i, jm)]) / dy2;

            double dTdx = (T_[index(ip, j)] - T_[index(im, j)]) / (2.0 * dx_);
            double dTdy = (T_[index(i, jp)] - T_[index(i, jm)]) / (2.0 * dy_);

            // Update state matrix
            T_new_[index(i, j)] = center - dt_ * (u_ * dTdx + v_ * dTdy) + dt_ * kappa_ * laplacian;
        }
    }

    // Zero-copy swap operation across our local region structures
    T_.swap(T_new_);
}

void Solver::run() {
    compute_timer_.reset();
    compute_timer_.start();

    for (int n = 0; n < steps_; ++n) {
        step();
    }

    compute_timer_.stop();
}

double Solver::compute_time() const {
    return compute_timer_.seconds();
}

void Solver::write_field(const std::string& filename) const {
    /*
       Right now, only Rank 0 captures files securely. 
       (Later we can expand this to unified MPI-IO files).
    */
    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("Could not open output file");
    }

    for (int i = 1; i <= local_nx_; ++i) {
        for (int j = 0; j < ny_; ++j) {
            file << T_[index(i, j)] << (j == ny_ - 1 ? "" : " ");
        }
        file << "\n";
    }
}