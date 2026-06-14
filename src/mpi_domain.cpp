#include "mpi_domain.hpp"

MPIDomain::MPIDomain(int global_nx, int global_ny, bool is_periodic, MPI_Comm comm)
    : comm_(comm),
      global_nx_(global_nx),
      global_ny_(global_ny),
      local_ny_(global_ny),
      is_periodic_(is_periodic)
{
    MPI_Comm_rank(comm_, &rank_); // "Who am I?" (0, 1, 2, 3...)
    MPI_Comm_size(comm_, &size_); // "How many total processes are running?"

    if (global_nx_ % size_ != 0) {
        throw std::runtime_error(
            "For this first MPI version, global_nx must be divisible by number of MPI ranks."
        );
    }

    // Horizontal domain slicing
    local_nx_ = global_nx_ / size_;
    start_x_  = rank_ * local_nx_;
    end_x_    = start_x_ + local_nx_ - 1;

    // Conditionally map neighbor Ranks based on the PBC toggle
    if (is_periodic_) {
        // CASE A: Wrapped Ring Topology (Periodic)
        left_rank_  = (rank_ == 0) ? (size_ - 1) : (rank_ - 1);
        right_rank_ = (rank_ == size_ - 1) ? 0 : (rank_ + 1);
    } else {
        // CASE B: Non-Periodic Wall Topology (Fixed boundaries)
        left_rank_  = (rank_ == 0) ? MPI_PROC_NULL : rank_ - 1;
        right_rank_ = (rank_ == size_ - 1) ? MPI_PROC_NULL : rank_ + 1;
    }
}

int MPIDomain::rank() const { return rank_; }
int MPIDomain::size() const { return size_; }

int MPIDomain::global_nx() const { return global_nx_; }
int MPIDomain::global_ny() const { return global_ny_; }

int MPIDomain::local_nx() const { return local_nx_; }
int MPIDomain::local_ny() const { return local_ny_; }

int MPIDomain::start_x() const { return start_x_; }
int MPIDomain::end_x() const { return end_x_; }

int MPIDomain::left_rank() const { return left_rank_; }
int MPIDomain::right_rank() const { return right_rank_; }

bool MPIDomain::is_periodic() const { return is_periodic_; }
MPI_Comm MPIDomain::comm() const { return comm_; }