
#include "benchmark.hpp"

#include <filesystem>
#include <fstream>

void append_benchmark(
    const std::string& filename,
    int nx,
    int ny,
    int steps,
    int mpi_rank,
    int omp_threads,
    double runtime,
    double compute_time,
    double mlups)
{
    bool write_header = !std::filesystem::exists(filename);

    std::ofstream file(
        filename,
        std::ios::app
    );

    if (write_header)
    {
        file
        << "nx,ny,steps,mpi_rank, threads,runtime,compute_time,mlups\n";
    }

    file
    << nx << ","
    << ny << ","
    << steps << ","
    << mpi_rank << ","
    << omp_threads << ","
    << runtime << ","
    << compute_time <<","
    << mlups << "\n";
}