#!/bin/bash
#SBATCH --job-name=climate_mpi       # Job name
#SBATCH --output=results/mpi_%j.log  # Standard output log
#SBATCH --error=results/mpi_%j.err   # Standard error log
#SBATCH --nodes=1                    # Request 1 node
#SBATCH --ntasks=2                   # Launch 2 MPI processes total
#SBATCH --ntasks-per-socket=1        # Bind 1 Rank to Socket 0 and 1 Rank to Socket 1
#SBATCH --cpus-per-task=16           # Dedicate 16 cores to each MPI Rank
#SBATCH --time=00:10:00              # Max walltime (10 minutes)
#SBATCH --partition=s384g9d          # Targeted cluster partition

# CRITICAL: Configure the compute node environment
module purge
module load tool/comp/gcc/13.1.0
module load tool/mpi/openmpi/4.1.5/gcc-13.1.0

echo "========================================================"
echo "Starting Multi-Socket MPI & Hybrid Runs on Intel Xeon"
echo "========================================================"

export OMP_PLACES=cores
export OMP_PROC_BIND=close

echo "--> RUN 1: Distributed MPI Scaling (-np 2, 1 Thread per Rank)"
export OMP_NUM_THREADS=1
mpirun -np 2 ./build/miniClimate --nx 2048 --ny 2048 --steps 500

echo "--> RUN 2: Production Hybrid Framework (-np 2, 16 Threads per Rank)"
export OMP_NUM_THREADS=16
mpirun -np 2 ./build/miniClimate --nx 2048 --ny 2048 --steps 500