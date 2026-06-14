#pragma once

#include <string>

void append_benchmark(
    const std::string& filename,
    int nx,
    int ny,
    int steps,
    int mpi_rank,
    int omp_threads,
    double runtime,
    double compute_time,
    double mlups
);