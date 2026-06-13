cmake_minimum_required(VERSION 3.16)
project(miniClimateHPC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Hint to CMake where Homebrew keeps libomp on Apple Silicon/Intel Macs
if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/libomp" "/usr/local/opt/libomp")
endif()

# Now find_package works flawlessly across Mac, Linux, and Windows
find_package(OpenMP REQUIRED)

# Define the executable target
add_executable(miniClimate
    src/main.cpp
    src/solver.cpp
    src/config.cpp
    src/benchmark.cpp

)

# Modern target-based linking (automatically handles flags, includes, and libs)
target_link_libraries(miniClimate
    PRIVATE
    OpenMP::OpenMP_CXX
)

# Target-specific include directories
target_include_directories(miniClimate 
    PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)

# Optimization and warning flags
target_compile_options(miniClimate PRIVATE -O3 -Wall -Wextra)