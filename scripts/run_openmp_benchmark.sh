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
STEPS=2000

OUT_DIR="results"
OUT="$OUT_DIR/openmp_benchmark.csv"

mkdir -p "$OUT_DIR"
rm -f "$OUT"

for threads in 1 2 4 8
do
    echo "=========================================="
    echo "Running OpenMP with OMP_NUM_THREADS=$threads"
    echo "=========================================="

    OMP_NUM_THREADS=$threads \
    mpirun -np 1 \
    $EXEC \
      --nx $NX \
      --ny $NY \
      --steps $STEPS \
      --benchmark "$OUT" \
      --output "$OUT_DIR/final_field_openmp_${threads}"
done

echo "Success! OpenMP benchmark results saved to: $OUT"