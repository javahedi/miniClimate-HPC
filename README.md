
# miniClimate-HPC

![CI](https://github.com/javahedi/QuantumTransportPP/actions/workflows/ci.yml/badge.svg)


`miniClimate-HPC` is a high-performance, physics-proxy mini-application implementing a **2D Finite-Difference Advection-Diffusion Solver**. It serves as an architectural proxy for computationally intensive stencil kernels found in production-grade climate, atmospheric, and ocean models (e.g., WRF, ICON).

The application solves the governing partial differential equation (PDE):

$$\frac{\partial T}{\partial t} + u \frac{\partial T}{\partial x} + v \frac{\partial T}{\partial y} = \kappa \left( \frac{\partial^2 T}{\partial x^2} + \frac{\partial^2 T}{\partial y^2} \right)$$

Using an explicit central-difference scheme in space and a forward Euler integration method in time.

---

## 🚀 Key HPC Engineering Highlights

* **Polymorphic Paradigm Switching:** Deployed a clean C++ abstract base interface (`Solver`) driving an automated runtime factory. The application dynamically provisions shared-memory execution (`SolverOpenMp`) or distributed-memory fabrics (`SolverMPI`) based entirely on the execution configuration.
* **1D Domain Decomposition with Non-Blocking Halo Exchanges:** Implemented a horizontal row slicing topology. Boundary conditions are synchronized across isolated memory address spaces utilizing highly optimized `MPI_Sendrecv` channels.
* **Zero-Overhead Periodic Boundaries:** Replaced expensive modulo arithmetic (`%`) inside critical compute loops with a dedicated runtime topology engine. Supports both **Periodic Boundary Conditions (PBC)** via wrapping ring configurations and closed-wall domains mapped natively to `MPI_PROC_NULL` for branches-free execution.
* **Data-Locality & Loop Optimization:** Leveraged OpenMP SIMD loop collapse routines (`collapse(2)`) to ensure high L1/L2 cache-line reuse, maximize memory bandwidth utilization, and minimize scheduling overhead.
* **Automated Production CI/CD:** Integrated a GitHub Actions pipeline running on virtualized Linux runners to dynamically compile, link parallel frameworks (`OpenMPI`, `libomp`), and run validation passes on every code push.

---

## 🗺️ Architecture & Memory Topology

The global grid is distributed across multi-node execution contexts using 1D row-wise domain decomposition. Each distributed process handles a localized horizontal slice wrapped in contiguous **Ghost/Halo Layers** to resolve spatial derivatives across network nodes.

```text
       GLOBAL 2D GRID                      LOCAL MEMORY ALLOCATION (RANK i)
    +--------------------+                      +--------------------+
    |      Row 0         |                      |     Ghost Top      |  <- Swapped via MPI
    +--------------------+                      +--------------------+
    |      Row 1         |                      |    Local Row 0     |
    +--------------------+  --- Sliced to --->  |    Local Row 1     |  <- Compute Loop Space
    |      Row 2         |    MPI Process i     |    Local Row 2     |
    +--------------------+                      +--------------------+
    |      Row 3         |                      |    Ghost Bottom    |  <- Swapped via MPI
    +--------------------+                      +--------------------+

```

---

## ⚙️ Compilation & Build Requirements

The framework utilizes modern target-based CMake paradigms to seamlessly chain include environments, flags, and link targets across **macOS (Apple Silicon)**, **Linux Enterprise Clusters**, and virtualized cloud runners.

### System Dependencies

* Modern C++17 compliant compiler (GCC $\ge$ 11, Clang $\ge$ 14)
* CMake $\ge$ 3.16
* OpenMP Development Libraries
* MPI Implementation (OpenMPI, MPICH, or Intel MPI)

### Build Instructions

```bash
# Clone the repository
git clone git@github.com:javahedi/miniClimate-HPC.git
cd project-miniClimate-HPC

# Configure and compile in Release mode
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

```

---

---

## 📊 Execution & Benchmark Harness

The application features automated validation and performance-tracking scripts that collect metrics across execution envelopes. Performance is strictly measured in **MLUPS** (Million Lattice Updates Per Second), capturing exact computational throughput.

### 1. Shared-Memory Execution (OpenMP)

```bash
./scripts/run_openmp_benchmark.sh

```

*Triggers cross-core multi-threaded execution over step increments, exporting scaling metrics to `results/openmp_benchmark.csv`.*

### 2. Distributed-Memory / Hybrid Execution (MPI)

```bash
./scripts/run_mpi_benchmark.sh

```

*Launches processing domains across active ranks. If `-np 1` is invoked, the factory falls back to the local OpenMP engine; if `-np > 1`, the distributed network engine boots up.*

### 3. Production Cluster Job Submission (Slurm Example)

To run on dedicated production cluster architecture like `auroram2`, batch submission scripts are used to handle hardware task placement:

```bash
# Submit the scaling matrices directly to the Slurm scheduler
sbatch scripts/submit_openmp.slurm
sbatch scripts/submit_mpi.slurm

```

---

# 📊 Performance Evaluation

The primary goal of miniClimate-HPC is not numerical accuracy, but the investigation of modern HPC software engineering techniques including OpenMP threading, MPI domain decomposition, communication overhead, and hybrid parallel execution.

All benchmark results reported below were obtained on a dedicated HPC cluster node.

---

## 🖥️ Benchmark Platform

| Attribute        | Specification            |
| ---------------- | ------------------------ |
| CPU              | Intel Xeon Platinum 9242 |
| Sockets          | 2                        |
| Physical Cores   | 96                       |
| Threads per Core | 1                        |
| NUMA Domains     | 4                        |
| Compiler         | GCC 13.1                 |
| MPI Library      | OpenMPI 4.1.5            |
| Build Type       | Release (-O3)            |

Benchmark configuration:

* Grid size: 2048 × 2048
* Time steps: 1000
* Three independent repetitions per measurement
* Runtime reported as wall-clock execution time
* Throughput reported in MLUPS (Million Lattice Updates Per Second)

---

# 🚀 OpenMP Strong Scaling

Configuration:

* MPI ranks = 1
* OpenMP threads varied from 1 to 16
* Thread affinity:

  * OMP_PLACES=cores
  * OMP_PROC_BIND=close

| Threads | Runtime (s) | MLUPS | Speedup | Efficiency |
| ------- | ----------: | ----: | ------: | ---------: |
| 1       |       65.46 |  64.1 |    1.00 |       100% |
| 2       |       34.16 | 122.8 |    1.92 |        96% |
| 4       |       17.58 | 238.6 |    3.72 |        93% |
| 8       |        8.86 | 473.3 |    7.39 |        92% |
| 16      |        4.63 | 905.7 |    14.1 |        88% |

### Analysis

The stencil kernel exhibits excellent shared-memory scalability. Runtime decreases nearly linearly with increasing thread count and achieves a speedup of approximately 14× on 16 CPU cores.

The sustained efficiency above 88% indicates:

* Good cache locality
* Low synchronization overhead
* Effective OpenMP loop parallelization
* High memory subsystem utilization

The maximum observed throughput exceeds 900 MLUPS.

---

# 🌐 MPI Strong Scaling

Configuration:

* OpenMP threads = 1
* MPI ranks varied from 1 to 32
* One-dimensional domain decomposition
* Halo communication implemented using MPI_Sendrecv

| MPI Ranks | Runtime (s) | MLUPS |
| --------- | ----------: | ----: |
| 1         |       65.43 |  64.1 |
| 2         |       26.18 | 160.2 |
| 4         |       28.11 | 149.2 |
| 8         |       32.12 | 130.6 |
| 16        |       64.13 |  65.4 |
| 32        |       130.4 |  32.2 |

### Analysis

The best performance is obtained with two MPI ranks.

Beyond this point, communication overhead increasingly dominates computation, causing a gradual loss of scalability. This behaviour is typical of stencil-based applications when the local sub-domain size becomes too small relative to halo-exchange costs.

The results highlight several future optimization targets:

* NUMA-aware rank placement
* Improved domain decomposition strategies
* Non-blocking communication
* Communication/computation overlap

---

# ⚡ Hybrid MPI + OpenMP

Configuration:

| MPI Ranks | OpenMP Threads |
| --------- | -------------- |
| 1         | 32             |
| 2         | 16             |
| 4         | 8              |
| 8         | 4              |
| 16        | 2              |

Benchmark results:

| MPI × OMP | Runtime (s) | MLUPS |
| --------- | ----------: | ----: |
| 1 × 32    |        2.53 |  1656 |
| 2 × 16    |       25.96 |   162 |
| 4 × 8     |       28.06 |   149 |
| 8 × 4     |       48.03 |    87 |
| 16 × 2    |       79.84 |    52 |

### Analysis

The best performance is achieved using a single MPI rank combined with 32 OpenMP threads.

The current hybrid implementation does not yet scale efficiently across increasing MPI rank counts, suggesting that communication costs dominate over computational gains in the present design.

This benchmark therefore serves as a useful case study for future optimization work focused on:

* Hybrid parallel execution
* MPI/OpenMP interaction
* NUMA locality
* Halo-exchange optimization
* Communication-aware decomposition strategies

---

# 🔬 Future Development Directions

The project is actively evolving toward a more realistic climate-model proxy application.

Planned improvements include:

* Two-dimensional Cartesian domain decomposition
* Non-blocking halo exchanges (MPI_Isend / MPI_Irecv)
* NUMA-aware rank placement
* SIMD vectorization analysis
* Roofline performance modelling
* Communication/computation overlap
* OpenMP task-based execution
* GPU offloading using OpenMP Target and OpenACC
* Performance portability studies across heterogeneous HPC systems



---

## 📂 Project Structure

```text
miniClimate-HPC/
├── .github/workflows/      # Automated CI/CD build actions
├── include/                # Modular Header layout
│   ├── solver.hpp          # Abstract core interface
│   ├── solver_openmp.hpp   # Shared-memory backend header
│   ├── solver_mpi.hpp      # Distributed network fabric header
│   └── mpi_domain.hpp      # Cartesian decomposition engine
├── src/                    # Source implementations
│   ├── main.cpp            # Application entrypoint & polymorphic factory
│   ├── solver_openmp.cpp   
│   ├── solver_mpi.cpp      
│   └── mpi_domain.cpp      
├── scripts/                # Automated benchmark loops & reporting scripts
├── results/                # Output field footprints and csv logs
└── CMakeLists.txt          # Target-based build setup

```


