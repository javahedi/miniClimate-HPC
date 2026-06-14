#!/bin/bash
#SBATCH --job-name=climate_omp       # Job name
#SBATCH --output=results/omp_%j.log  # Standard output log (%j inserts JobID)
#SBATCH --error=results/omp_%j.err   # Standard error log
#SBATCH --nodes=1                    # Request exactly 1 node
#SBATCH --ntasks=1                   # 1 MPI process (Rank 0)
#SBATCH --cpus-per-task=16           # Request 16 cores on that node
#SBATCH --time=00:10:00              # Max walltime (10 minutes)
#SBATCH --partition=s384g9d          # Targeted cluster partition

# CRITICAL: Configure the compute node environment
module purge                          # Clear any inheriting default system paths
module load tool/comp/gcc/13.1.0      # Load your exact compiler runtime
module load tool/mpi/openmpi/4.1.5/gcc-13.1.0

echo "========================================================"
echo "Starting Pure OpenMP Thread Scaling on Intel Xeon Gold"
echo "========================================================"

for t in 1 2 4 8 16; do
    echo "----------------------------------------------------"
    echo "Running with OMP_NUM_THREADS=$t"
    echo "----------------------------------------------------"
    
    export OMP_NUM_THREADS=$t
    export OMP_PLACES=cores
    export OMP_PROC_BIND=close

    ./build/miniClimate --nx 2048 --ny 2048 --steps 500
done