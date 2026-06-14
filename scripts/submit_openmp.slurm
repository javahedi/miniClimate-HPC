#!/bin/bash
#SBATCH --job-name=climate_omp
#SBATCH --output=results/logs/omp_%j.out
#SBATCH --error=results/logs/omp_%j.err
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --exclusive
#SBATCH --time=00:20:00
#SBATCH --partition=s384g9d

set -euo pipefail

module purge
module load tool/comp/gcc/13.1.0
module load tool/mpi/openmpi/4.1.5/gcc-13.1.0

mkdir -p results/logs results/fields

echo "========================================================"
echo "miniClimate-HPC OpenMP strong-scaling benchmark"
echo "Job ID          : ${SLURM_JOB_ID}"
echo "Node            : ${SLURM_NODELIST}"
echo "CPUs/task       : ${SLURM_CPUS_PER_TASK}"
echo "Date            : $(date)"
echo "========================================================"



#echo "========================================================" 
#echo "Building code" 
#echo "========================================================" 
#rm -rf build 
#cmake -S . -B build -DCMAKE_BUILD_TYPE=Release #cmake --build build -j ${SLURM_CPUS_PER_TASK}


lscpu | egrep "Model name|CPU\\(s\\)|Core\\(s\\)|Socket\\(s\\)|Thread\\(s\\)|NUMA"

EXEC=./build/miniClimate

if [ ! -x "$EXEC" ]; then
    echo "Error: executable $EXEC not found or not executable"
    exit 1
fi

NX=2048
NY=2048
STEPS=1000
REPEATS=3

OUT=results/openmp_strong_${SLURM_JOB_ID}.csv
rm -f "$OUT"

export OMP_PLACES=cores
export OMP_PROC_BIND=close

for t in 1 2 4 8 16
do
    for rep in $(seq 1 ${REPEATS})
    do
        echo "OpenMP: threads=${t}, repeat=${rep}"

        export OMP_NUM_THREADS=${t}

        mpirun --bind-to none -np 1 ${EXEC} \
            --nx ${NX} \
            --ny ${NY} \
            --steps ${STEPS} \
            --benchmark "$OUT" \
            --output "results/fields/openmp_t${t}_rep${rep}"
    done
done

echo "Benchmark CSV: $OUT"
cat "$OUT"