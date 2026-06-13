#!/bin/bash

set -e

EXEC=./build/miniClimate
NX=1024
NY=1024
STEPS=2000

OUT=results/openmp_benchmark.csv

rm -f benchmark.csv
rm -f "$OUT"

for threads in 1 2 4 8
do
    echo "Running with OMP_NUM_THREADS=$threads"

    OMP_NUM_THREADS=$threads \
    $EXEC \
      --nx $NX \
      --ny $NY \
      --steps $STEPS \
      --output results/final_field_${threads}.dat
done

mv benchmark.csv "$OUT"

echo "Benchmark saved to $OUT"