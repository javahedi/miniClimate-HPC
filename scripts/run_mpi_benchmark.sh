#!/bin/bash
set -e

if [ ! -d "build" ]; then
    echo "Build directory not found. Creating and compiling..."
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
fi

echo "Ensuring project is compiled..."
cmake --build build -j

EXEC=./build/miniClimate
NX=1024
NY=1024
STEPS=500

OUT_DIR="results"
OUT="$OUT_DIR/mpi_benchmark.csv"

mkdir -p "$OUT_DIR"
rm -f "$OUT"

export OMP_NUM_THREADS=1

for ranks in 1 2 4 8
do
    echo "=========================================="
    echo "Running pure MPI with -np $ranks"
    echo "=========================================="

    mpirun -np $ranks \
      $EXEC \
        --nx $NX \
        --ny $NY \
        --steps $STEPS \
        --benchmark "$OUT" \
        --output "$OUT_DIR/final_field_mpi_${ranks}"
done

echo "Success! Pure MPI benchmark results saved to: $OUT"