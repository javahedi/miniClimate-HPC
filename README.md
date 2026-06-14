
# miniClimate-HPC

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

## 🖥️ Target Benchmarking Hardware Specifications

To evaluate the scalability, memory-subsystem bottlenecks, and multi-paradigm performance of the stencil solver, the application is systematically profiled across two highly distinct microarchitectures:

### 1. Local Development Platform: Apple M1 (Asymmetric SoC)

A heterogeneous, low-latency ARM-based microarchitecture characterized by asymmetric core clusters and shared L2 cache rings.

| Attribute | Specification | Engineering Significance |
| --- | --- | --- |
| **Processor Type** | Apple M1 (ARM64) | Single-die System-on-Chip (SoC) |
| **Total Cores** | 8 Physical Cores | Configured as 4 Performance + 4 Efficiency Cores |
| **Performance Cores (`perflevel0`)** | 4 Cores @ 3.2 GHz | Handles heavy vector/floating-point operations |
| **Efficiency Cores (`perflevel1`)** | 4 Cores @ 2.1 GHz | Handles background threads at reduced power envelopes |
| **L1 Cache (Perf Cluster)** | 192 KB Inst / 128 KB Data per core | Exceptionally large L1, ideal for single-line stencil caching |
| **L1 Cache (Eff Cluster)** | 131 KB Inst / 64 KB Data per core | Reduced size; potential source of thread imbalance |
| **L2 Cache (Perf Cluster)** | 12 MB (Shared across 4 Perf Cores) | Allows localized multi-threaded grids to fit entirely in fast cache |
| **L2 Cache (Eff Cluster)** | 4 MB (Shared across 4 Eff Cores) | Dedicated slower pool; induces synchronization penalties if over-subscribed |

### 2. Production Cluster Platform: `auroram2` (Enterprise HPC Node)

A heavy-duty, dual-socket x86_64 server node designed for distributed-memory applications, featuring high-capacity AVX-512 execution pipelines and Non-Uniform Memory Access (NUMA).

| Attribute | Specification | Engineering Significance |
| --- | --- | --- |
| **Processor Model** | Intel(R) Xeon(R) Gold 6426Y (Sapphire Rapids) | Server-grade high-performance processing fabric |
| **Sockets Installed** | 2 Independent Sockets | Dual-processor system interconnect via high-speed Intel UPI links |
| **Physical Cores** | 32 Cores Total (16 Cores per socket) | True physical parallelism across separate compute sockets |
| **Hardware Threads** | 64 Logical Cores (Simultaneous Multithreading / HT) | 2 Hardware threads per physical core |
| **Total L1 Data Cache** | 1.5 MiB (32 individual instances of 48 KiB) | Dedicated per-core low-latency storage for array stencils |
| **Total L2 Cache** | 64 MiB (32 individual instances of 2 MiB) | Massive per-core unified cache preventing RAM thrashing |
| **Total L3 Cache (LLC)** | 75 MiB Shared (2 instances of 37.5 MiB) | Last-Level Cache mapped per socket domain |
| **NUMA Configuration** | 2 Discrete NUMA Nodes | **Node 0:** Cores 0-15, 32-47 | **Node 1:** Cores 16-31, 48-63 |
| **Vector Instruction Sets** | AVX-512, AMX (Advanced Matrix Extensions) | Capable of massive SIMD parallel register packing for grid math |

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

## 📈 Performance Profiling & Scaling Analysis

### 🍏 Platform 1: Apple M1 Local Benchmark Results

*Test Parameters: Grid Size = $1024 \times 1024$, Steps = 2000 (OpenMP) / 500 (MPI)*

#### Shared-Memory Scaling (Pure OpenMP)

| Threads | Runtime (s) | Throughput (MLUPS) | Parallel Speedup | Parallel Efficiency |
| --- | --- | --- | --- | --- |
| **1** | 10.54s | 199.02 | 1.00x (Baseline) | 100.0% |
| **2** | 5.51s | 380.37 | 1.91x | 95.5% |
| **4** | 3.33s | 629.23 | 3.16x | 79.0% |
| **8** | 2.90s | 723.64 | 3.63x | 45.4% |

> 💡 **Hardware Insight (The Heterogeneous Core Wall):** Scaling from 1 to 4 threads yields solid parallel efficiency ($\ge 79\%$). However, moving from 4 to 8 threads results in severe scalability degradation (efficiency drops to $45.4\%$). This maps directly to the Apple M1's asymmetric topology: the first 4 threads occupy the Performance Cores (3.2 GHz, 12 MB shared L2), while threads 5-8 are forced onto the Efficiency Cores (2.1 GHz, 4 MB shared L2). The fast threads rapidly complete their loops and sit idle at the implicit OpenMP barrier waiting for the slower efficiency threads to finish.

#### Distributed-Memory Scaling (Pure MPI)

| Ranks (`-np`) | Runtime (s) | Throughput (MLUPS) | Relative Speedup vs `-np 1` | Scaling Behavior |
| --- | --- | --- | --- | --- |
| **1** | 2.59s | 202.78 | 1.00x (Baseline) | Cascades to Single-Thread OpenMP |
| **2** | 0.60s | 874.61 | **4.31x** | **Superlinear Scaling** |
| **4** | 0.31s | 1669.84 | **8.23x** | **Superlinear Peak** |
| **8** | 0.83s | 629.69 | 3.10x | Context-Switching Saturation |

> 💡 **Hardware Insight (L2 Cache Resonance):** The transition from `-np 1` to `-np 2` and `-np 4` exhibits remarkable superlinear speedup ($4.31\text{x}$ and $8.23\text{x}$). At `-np 1`, the full $1024 \times 1024$ double-precision array configuration consumes approximately $16\text{ MB}$ total, exceeding the $12\text{ MB}$ shared L2 cache pool and forcing the processor to throttle on main system memory bus latency. At `-np 4`, the grid decomposition partitions the subgrids down to $\approx 2\text{ MB}$ per rank, enabling the working set to fit **entirely** inside the fast L2 cache pool, eliminating RAM bus penalties entirely.

---

### 🏢 Platform 2: `auroram2` Enterprise Cluster Benchmark Results

*Test Parameters: Grid Size = $2048 \times 2048$, Steps = 500*

#### High-Efficiency Thread Scaling (Pure OpenMP)

*Configuration: 1 MPI Rank, bound securely to a single 16-core physical socket domain.*

| Threads | Runtime (s) | Throughput (MLUPS) | Speedup | Parallel Efficiency |
| --- | --- | --- | --- | --- |
| **1** | 33.01s | 63.53 | 1.00x (Baseline) | 100.0% |
| **2** | 17.08s | 122.78 | 1.93x | 96.5% |
| **4** | 8.80s | 238.44 | 3.75x | 93.8% |
| **8** | 4.43s | 473.50 | 7.45x | 93.1% |
| **16** | 2.31s | 907.13 | **14.28x** | **89.3%** |

> 💡 **Hardware Insight (Explicit Thread-to-Core Pinning):** By enforcing strict hardware affinity layers (`OMP_PLACES=cores` and `OMP_PROC_BIND=close`), the thread scheduler pinned individual execution streams directly to the physical silicon. This prevented core-hopping and maximized L1/L2 cache locality, allowing performance to scale linearly up to an impressive **907.13 MLUPS** with near-zero resource contention.

#### Multi-Socket & Hybrid Paradigm Traps

*Configuration: 2 MPI Ranks distributed across 2 separate sockets via Slurm layout.*

* **Pure Distributed Run (-np 2, 1 Thread/Rank):** Runtime: **13.10s** | Throughput: **160.03 MLUPS**
* **Hybrid Execution Run (-np 2, 16 Threads/Rank):** Runtime: **13.05s** | Throughput: **160.64 MLUPS**

> 💡 **Post-Mortem Profiling Insight:** Initial benchmark iterations revealed an allocation plateau where adding 16 OpenMP threads per MPI rank yielded zero runtime improvement. Profiling isolated this block to a missing parallel region loop-collapse within the distributed solver's internal grid updates. This highlights a vital HPC structural constraint: distributed domain decomposition and ghost-cell halo exchanges (`MPI_Sendrecv`) function independently from shared-memory vector pools. To unlock true hybrid scaling across multi-socket systems, OpenMP loop directives must actively parallelize the internal sub-grid iterations within each independent MPI memory space.

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
```

