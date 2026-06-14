Rewriting your `README.md` from scratch is the absolute best way to turn this project into a high-powered portfolio piece. Recruiters and team leads in the High-Performance Computing (HPC) space don't just look for working code—they want to see that you understand **hardware architecture, memory hierarchies, parallel paradigms, and performance profiling.**

Your current README reads like a simple student assignment. To attract top-tier HPC recruiters, we need to restructure it to look like a professional, battle-tested scientific mini-app.

Here is a completely overhauled, production-grade `README.md` designed to make an impact.

---

# # miniClimate-HPC

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
cd project-miniClimte-HPC

# Configure and compile in Release mode
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

```

---

## 📊 Execution & Benchmark Harness

The application features automated validation and performance-tracking scripts that collect metrics across execution envelopes. Performance is strictly measured in **MLUPS** (Million Lattice Updates Per Second), capturing exact computational throughput.

### 1. Pure Shared-Memory Execution (OpenMP)

```bash
./scripts/run_openmp_benchmark.sh

```

*Triggers cross-core multi-threaded execution over step increments, exporting scaling metrics to `results/openmp_benchmark.csv`.*

### 2. Distributed-Memory / Hybrid Execution (MPI)

```bash
./scripts/run_mpi_benchmark.sh

```

*Launches processing domains across active ranks. If `-np 1` is invoked, the factory falls back to the local OpenMP engine; if `-np > 1`, the distributed network engine boots up.*

Manual command-line execution example:

```bash
# Run a hybrid job using 4 MPI processes, pinning 4 OpenMP threads per process
OMP_NUM_THREADS=4 mpirun -np 4 ./build/miniClimate --nx 1024 --ny 1024 --steps 1000

```

---

## 📈 Performance Profile Insights (Apple M1 Architecture Example)

### Hardware-Aware Insights Found During Profiling:

* **Superlinear Scaling via L2 Cache Resonance:** When transitioning from 1 Rank to 2 and 4 Ranks under pure MPI, throughput scales *superlinearly* (leaping from **200 MLUPS** to **1,381 MLUPS**). This occurs because slicing the problem grid allows the local matrix footprint to fit entirely within the fast L2 hardware cache, eliminating RAM bus latency bottlenecks.
* **Asymmetric Core Bottlenecks:** Scaling OpenMP blindly to 8 threads on standard Apple Silicon runs into a performance plateau. This demonstrates the structural difference between high-performance cores and energy-efficient cores, where slow-running threads stall performance at OpenMP synchronization barriers.

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

