cmake_minimum_required(VERSION 3.14)
project(NBodySimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(ENABLE_AUTH "Enable authentication (requires libpqxx)" ON)

# Optimization flags
if(MSVC)
    add_compile_options(/O2 /arch:AVX2)
else()
    add_compile_options(-O3 -march=native -pthread)
endif()

# Fetch SFML 3
include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        3.0.0
)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SFML)

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
    src/Menu.cpp
)

if(ENABLE_AUTH)
    list(APPEND SOURCES src/AuthManager.cpp)
endif()

# Executable
add_executable(nbody_simulation ${SOURCES})

# Link libraries
target_link_libraries(nbody_simulation PRIVATE SFML::Graphics SFML::Window SFML::System)

if(ENABLE_AUTH)
    # On Linux, usually linked via pkg-config or standard paths.
    # On Windows, ensure 'pqxx' and 'pq' are available (e.g. via vcpkg).
    target_link_libraries(nbody_simulation PRIVATE pqxx pq)
    target_compile_definitions(nbody_simulation PRIVATE ENABLE_AUTH)
endif()

if(NOT WIN32)
    target_link_libraries(nbody_simulation PRIVATE pthread)
endif()

# Install target
install(TARGETS nbody_simulation DESTINATION bin)
