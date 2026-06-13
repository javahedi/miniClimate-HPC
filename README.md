# miniClimate-HPC

A compact HPC mini-application implementing a 2D finite-difference diffusion solver.

The project is designed as a proxy for stencil-based numerical kernels commonly found in atmospheric and ocean models.

## Features

- Modern C++17
- CMake build system
- 2D finite-difference stencil
- Periodic boundary conditions
- OpenMP parallelisation
- Runtime benchmarking
- MLUPS performance metric
- Automated benchmark scripts

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Run

```bash
./build/miniClimate \
    --nx 1024 \
    --ny 1024 \
    --steps 2000
```

## OpenMP Benchmark

```bash
./scripts/benchmark_openmp.sh
```

Generate speedup plot:

```bash
python3 scripts/plot_openmp.py
```

## Example Benchmark Metrics

- Runtime (s)
- MLUPS (Million Lattice Updates Per Second)
- OpenMP speedup

## Project Structure

```text
miniClimate-HPC/
├── src/
├── scripts/
├── results/
├── CMakeLists.txt
└── README.md
```
