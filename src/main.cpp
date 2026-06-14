#include "config.hpp"
#include "solver.hpp"
#include "benchmark.hpp"
#include "mpi_domain.hpp"

#include <mpi.h>
#include <omp.h>
#include <chrono>
#include <iostream>

int main(int argc, char** argv)
{
    // 1. Fire up the MPI environment before any processing takes place
    MPI_Init(&argc, &argv);
    
    try {
        Config cfg = parse_args(argc, argv);

        // 2. Instantiate our domain mapping (Defaults to Periodic = true)
        MPIDomain domain(cfg.nx, cfg.ny);

        // 3. ONLY Rank 0 should print the configuration block
        if (domain.rank() == 0) {
            print_config(cfg);
        }

        // 4. Synchronization Barrier: Force all ranks to reach this line 
        // before dumping their structural ownership reports to stdout
        MPI_Barrier(domain.comm());

        std::cout << "[MPI Setup] Rank " << domain.rank() 
                  << " / " << domain.size()
                  << " handles global rows " << domain.start_x() 
                  << " to " << domain.end_x() << "\n";

        // 5. Synchronization Barrier: Make sure printing finishes before 
        // proceeding to legacy physics calculations
        MPI_Barrier(domain.comm());

        /*
           ⚠️ TEMPORARY DESIGN WARNING:
           Our Solver object doesn't accept 'domain' yet. For this brief test,
           we let the physics loop run as-is, but we strictly guard file generation
           so that only Rank 0 touches the storage systems.
        */
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

        // 6. Only Rank 0 logs global throughput metrics to console and CSV files
        if (domain.rank() == 0) {
            std::cout << "\n====================================\n";
            std::cout << "Global Runtime : " << runtime << " s\n";
            std::cout << "Global MLUPS   : " << mlups << "\n";
            std::cout << "====================================\n";

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
    }
    catch (const std::exception& e) {
        std::cerr << "Process Rank Error occurred: " << e.what() << "\n";
        // If an execution thread fails, abort all cluster nodes immediately
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }

    // 7. Gracefully shut down the distributed system layers
    MPI_Finalize();
    return 0;
}