#pragma once 

#include <mpi.h>
#include <stdexcept>

/*
  MPIDomain stores information about how the global 2D grid
  is split between MPI ranks.


    +------------------+
    | ghost_top        |
    +------------------+
    | row0             |
    | row1             |
    | row2             |
    | row3             |
    +------------------+
    | ghost_bottom     |
    +------------------+

  For now we use 1D domain decomposition in the x-direction.

  Example:
      global_nx = 16
      size      = 4

  Then each rank gets 4 rows:

      rank 0 -> rows 0..3
      rank 1 -> rows 4..7
      rank 2 -> rows 8..11
      rank 3 -> rows 12..15

  Later each rank will also allocate two ghost rows:
      one above and one below the local physical domain.
*/



class MPIDomain {
public:
    // Added is_periodic flag to the constructor
    MPIDomain(int global_nx, int global_ny, bool is_periodic = true, MPI_Comm comm = MPI_COMM_WORLD);

    int rank() const;
    int size() const;

    int global_nx() const;
    int global_ny() const;

    int local_nx() const;
    int local_ny() const;

    int start_x() const;
    int end_x() const;

    int left_rank() const;
    int right_rank() const;

    bool is_periodic() const; // Getter to check boundary profile
    MPI_Comm comm() const;

private:
    MPI_Comm comm_;

    int rank_;
    int size_;

    int global_nx_;
    int global_ny_;

    int local_nx_;
    int local_ny_;

    int start_x_;
    int end_x_;

    int left_rank_;
    int right_rank_;
    
    bool is_periodic_; // Tracks our boundary choice
};