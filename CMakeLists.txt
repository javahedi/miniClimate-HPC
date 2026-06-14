cmake_minimum_required(VERSION 3.16)
project(miniClimateHPC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Platform-specific hints for Homebrew paths on macOS
if(APPLE)
    # Search paths for OpenMP and OpenMPI
    list(APPEND CMAKE_PREFIX_PATH 
        "/opt/homebrew/opt/libomp"
        "/opt/homebrew/opt/open-mpi"
        "/usr/local/opt/libomp"
        "/usr/local/opt/open-mpi"
    )
endif()

# 2. Find Parallel Frameworks
find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED)

# 3. Define the Executable Target
add_executable(miniClimate
    src/main.cpp
    src/config.cpp
    src/benchmark.cpp
    src/mpi_domain.cpp
    src/solver_openmp.cpp
    src/solver_mpi.cpp
)

# 4. Target-Based Linking
# This automatically chains the include directories, compiler options, and library links
target_link_libraries(miniClimate
    PRIVATE
    OpenMP::OpenMP_CXX
    MPI::MPI_CXX
)
# Target-specific include directories
target_include_directories(miniClimate 
    PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)

# Optimization and warning flags
target_compile_options(miniClimate PRIVATE -O3 -Wall -Wextra)



