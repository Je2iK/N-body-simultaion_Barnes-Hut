cmake_minimum_required(VERSION 3.10)
project(NBodySimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -pthread")

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/Utils.cpp
    src/Cell.cpp
    src/BarnesHutSimulator.cpp
    src/BruteForceSimulator.cpp
    src/Benchmark.cpp
)

# Executable
add_executable(nbody_simulation ${SOURCES})

# Link libraries
target_link_libraries(nbody_simulation sfml-graphics sfml-window sfml-system pthread)

# Install target
install(TARGETS nbody_simulation DESTINATION bin)
