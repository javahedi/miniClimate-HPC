#!/bin/bash
#SBATCH --job-name=climate_mpi_hybrid
#SBATCH --output=results/logs/mpi_hybrid_%j.out
#SBATCH --error=results/logs/mpi_hybrid_%j.err
#SBATCH --nodes=1
#SBATCH --ntasks=32
#SBATCH --cpus-per-task=1
#SBATCH --exclusive
#SBATCH --time=00:30:00
#SBATCH --partition=s384g9d

set -euo pipefail

module purge
module load tool/comp/gcc/13.1.0
module load tool/mpi/openmpi/4.1.5/gcc-13.1.0

mkdir -p results/logs results/fields

echo "========================================================"
echo "miniClimate-HPC MPI / Hybrid benchmark"
echo "Job ID      : ${SLURM_JOB_ID}"
echo "Node        : ${SLURM_NODELIST}"
echo "Date        : $(date)"
echo "========================================================"

lscpu | egrep "Model name|CPU\\(s\\)|Core\\(s\\)|Socket\\(s\\)|Thread\\(s\\)|NUMA"

EXEC=./build/miniClimate

if [ ! -x "$EXEC" ]; then
    echo "Error: executable $EXEC not found or not executable"
    echo "Build the code first on the login node:"
    echo "  rm -rf build"
    echo "  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release"
    echo "  cmake --build build -j 16"
    exit 1
fi

NX=2048
NY=2048
STEPS=1000
REPEATS=3

MPI_OUT=results/mpi_strong_${SLURM_JOB_ID}.csv
HYBRID_OUT=results/hybrid_${SLURM_JOB_ID}.csv

rm -f "$MPI_OUT" "$HYBRID_OUT"

export OMP_PLACES=cores
export OMP_PROC_BIND=close

echo "========================================================"
echo "Pure MPI strong scaling"
echo "OMP_NUM_THREADS=1"
echo "========================================================"

export OMP_NUM_THREADS=1

for ranks in 1 2 4 8 16 32
do
    for rep in $(seq 1 ${REPEATS})
    do
        echo "Pure MPI: ranks=${ranks}, repeat=${rep}"

        mpirun --bind-to none -np ${ranks} \
            ${EXEC} \
            --nx ${NX} \
            --ny ${NY} \
            --steps ${STEPS} \
            --benchmark "$MPI_OUT" \
            --output "results/fields/mpi_r${ranks}_rep${rep}"
    done
done

echo "========================================================"
echo "Hybrid MPI + OpenMP benchmark"
echo "Total workers approximately 32 physical cores"
echo "========================================================"

for cfg in "1 32" "2 16" "4 8" "8 4" "16 2" "32 1"
do
    set -- $cfg
    ranks=$1
    threads=$2

    export OMP_NUM_THREADS=${threads}

    for rep in $(seq 1 ${REPEATS})
    do
        echo "Hybrid: ranks=${ranks}, threads=${threads}, repeat=${rep}"

        mpirun --bind-to none -np ${ranks} \
            ${EXEC} \
            --nx ${NX} \
            --ny ${NY} \
            --steps ${STEPS} \
            --benchmark "$HYBRID_OUT" \
            --output "results/fields/hybrid_r${ranks}_t${threads}_rep${rep}"
    done
done

echo "========================================================"
echo "Finished"
echo "Pure MPI CSV : $MPI_OUT"
echo "Hybrid CSV   : $HYBRID_OUT"
echo "========================================================"

echo "Pure MPI results:"
cat "$MPI_OUT"

echo "Hybrid results:"
cat "$HYBRID_OUT"