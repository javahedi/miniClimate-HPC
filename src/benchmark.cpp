
#include "benchmark.hpp"

#include <filesystem>
#include <fstream>

void append_benchmark(
    const std::string& filename,
    int nx,
    int ny,
    int steps,
    int threads,
    double runtime,
    double mlups)
{
    bool write_header =
        !std::filesystem::exists(filename);

    std::ofstream file(
        filename,
        std::ios::app
    );

    if (write_header)
    {
        file
        << "nx,ny,steps,threads,runtime,mlups\n";
    }

    file
    << nx << ","
    << ny << ","
    << steps << ","
    << threads << ","
    << runtime << ","
    << mlups << "\n";
}