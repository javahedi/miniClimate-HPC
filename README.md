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




## Architectural Design: 1D Domain Decomposition & MPI Halo Exchange

This document acts as the technical specifications manual for upgrading the `miniClimate-HPC` solver from shared-memory OpenMP to distributed-memory MPI (Message Passing Interface). 

---

### 1. Core Concepts: Shared vs. Distributed Memory

Before looking at the grid math, we distinguish the architectural transition of our parallel frameworks:

| Feature | OpenMP (Current) | MPI (Target Upgrade) |
| :--- | :--- | :--- |
| **Memory Architecture** | **Shared Memory**: All threads read/write to the same physical RAM layout. | **Distributed Memory**: Each process (Rank) has isolated RAM. Processors cannot touch each other's memory directly. |
| **Execution Model** | Single process spawns multiple lightweight threads. | Multiple independent processes running the same executable binaries (SPMD). |
| **Communication** | Implicit via shared variables and pointers. | Explicit via **Message Passing** (sending and receiving buffers over a network fabrics/interconnect). |
| **Scalability** | Limited to a single motherboard socket/node (e.g., 8–64 cores). | Scales to massive supercomputing clusters across thousands of discrete compute nodes. |

---

### 2. 1D Domain Decomposition

To scale our $1024 \times 1024$ grid across distributed nodes, we implement **1D Row-Wise Decomposition** along the outer $x$-axis (`i` dimension). 

If the global grid size is $NX \times NY$, and we launch the application with $P$ MPI Ranks, the domain is sliced horizontally:

$$\text{local\_nx} = \frac{NX}{P}$$

#### Example Distribution ($1024 \times 1024$ Grid across 4 Ranks)
* **Rank 0** : Manages Global Rows $0$ to $255$ ($\text{local\_nx} = 256$)
* **Rank 1** : Manages Global Rows $256$ to $511$ ($\text{local\_nx} = 256$)
* **Rank 2** : Manages Global Rows $512$ to $767$ ($\text{local\_nx} = 256$)
* **Rank 3** : Manages Global Rows $768$ to $1023$ ($\text{local\_nx} = 256$)

---

### 3. The Local Grid Data Map (Ghost & Halo Rows)

Because our finite-difference stencil computes spatial derivatives using neighboring rows ($i+1$ and $i-1$), local boundaries require data owned by neighboring hardware ranks. We resolve this dependency by wrapping our local data domain in **Ghost Rows** (Halo Layers).

#### Memory Layout for a Local MPI Rank
Instead of allocating just its active rows, each rank allocates an extended buffer containing $\text{local\_nx} + 2$ total rows.

```text
       Row Index     Type of Row                      Source / Destination Logic
    ┌─────────────┼──────────────────────────────────┼──────────────────────────────┐
-1  │  Index 0    │ 👻 Top Ghost Row (Halo)          │ Received via MPI from Up     │
    ├─────────────┼──────────────────────────────────┼──────────────────────────────┤
 0  │  Index 1    │ 🏠 First Local Row (Boundary)    │ Sent via MPI to Up           │
 1  │  Index 2    │ 📦 Inner Local Row               │ Computed Locally             │
    │    ...      │ 📦 ...                           │ ...                          │
    │  Index N-2  │ 📦 Inner Local Row               │ Computed Locally             │
N-1 │  Index N-1  │ 🏠 Last Local Row (Boundary)     │ Sent via MPI to Down         │
    ├─────────────┼──────────────────────────────────┼──────────────────────────────┤
 N  │  Index N    │ 👻 Bottom Ghost Row (Halo)       │ Received via MPI from Down   │
    └─────────────┴──────────────────────────────────┴──────────────────────────────┘
