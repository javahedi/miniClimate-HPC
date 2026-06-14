#include "config.hpp"
#include "solver.hpp"
#include "benchmark.hpp"

#include <omp.h>
#include <chrono>
#include <iostream>

int main(int argc, char** argv)
{
   
    try {

        Config cfg = parse_args(argc, argv);

        print_config(cfg);

        Solver solver(
            cfg.nx,
            cfg.ny,
            cfg.steps,
            cfg.u,
            cfg.v,
            cfg.dt,
            cfg.kappa
        );

        solver.initialize();

        auto start = std::chrono::high_resolution_clock::now();

        solver.run();
        double compute_time = solver.compute_time();


        auto end = std::chrono::high_resolution_clock::now();

        double runtime = std::chrono::duration<double>(end - start).count();

        long long updates = static_cast<long long>(cfg.nx) * cfg.ny * cfg.steps;

        double mlups = updates / runtime / 1e6;

        std::cout << "\nRuntime : " << runtime << " s\n";
        // Million Lattice Updates Per Second
        std::cout << "MLUPS   : " << mlups << "\n";

        int threads = omp_get_max_threads();

        append_benchmark(
            "benchmark.csv",
            cfg.nx,
            cfg.ny,
            cfg.steps,
            threads,
            runtime,
            compute_time,
            mlups
        );

        solver.write_field(cfg.output);
    }
    catch (const std::exception& e) {

        std::cerr
            << "Error: "
            << e.what()
            << "\n";

        return 1;
    }

    return 0;
}

/*
./miniClimate \
    --nx 512 \
    --ny 512 \
    --steps 1000 \
    --dt 1e-6 \
    --kappa 0.01
*/

/*
OpenMP

Line command--> 
# Run with 2 threads
OMP_NUM_THREADS=2 ./miniClimate --nx 512 --ny 512 --steps 1000 --dt 1e-6 --kappa 0.01

# Run with 4 threads
OMP_NUM_THREADS=4 ./miniClimate --nx 512 --ny 512 --steps 1000 --dt 1e-6 --kappa 0.01

# Run with 8 threads
OMP_NUM_THREADS=8 ./miniClimate --nx 512 --ny 512 --steps 1000 --dt 1e-6 --kappa 0.01


or Export for the Session

export OMP_NUM_THREADS=4

# All subsequent runs will now use 4 threads automatically
./miniClimate --nx 512 --ny 512 --steps 1000 --dt 1e-6 --kappa 0.01
*/