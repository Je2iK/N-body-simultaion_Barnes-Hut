cmake_minimum_required(VERSION 3.14)
project(NBodySimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Системные зависимости
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)

find_package(X11 REQUIRED)
if(NOT X11_FOUND)
    message(FATAL_ERROR "X11 libraries not found")
endif()

pkg_check_modules(XRANDR REQUIRED xrandr)
pkg_check_modules(XCURSOR REQUIRED xcursor)
pkg_check_modules(XI REQUIRED xi)

# Optimization flags
if(MSVC)
    add_compile_options(/O2 /arch:AVX2)
else()
    add_compile_options(-O3 -march=native -pthread)
endif()

# Fetch dependencies
include(FetchContent)

# Загрузка SFML 3
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        3.0.0
)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)

# Загрузка libpqxx для аутентификации
if(ENABLE_AUTH)
    FetchContent_Declare(
        libpqxx
        GIT_REPOSITORY https://github.com/jtv/libpqxx.git
        GIT_TAG        7.9.0
    )
    set(SKIP_BUILD_TEST ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(SFML)
if(ENABLE_AUTH)
    FetchContent_MakeAvailable(libpqxx)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

# Исходные файлы
set(SOURCES
    src/main.cpp
    src/Utils.cpp
    src/Cell.cpp
    src/BarnesHutSimulator.cpp
    src/BruteForceSimulator.cpp
    src/Benchmark.cpp
    src/Menu.cpp
    src/QueryLoader.cpp
)

if(ENABLE_AUTH)
    list(APPEND SOURCES src/AuthManager.cpp)
endif()

# Executable
add_executable(nbody_simulation ${SOURCES})

# Target-specific include directories
target_include_directories(nbody_simulation PRIVATE
    ${X11_INCLUDE_DIR}
    ${XRANDR_INCLUDE_DIRS}
    ${XCURSOR_INCLUDE_DIRS}
    ${XI_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(nbody_simulation PRIVATE 
    SFML::Graphics SFML::Window SFML::System
    OpenGL::GL
    Freetype::Freetype
    ${X11_LIBRARIES}
    ${XRANDR_LIBRARIES}
    ${XCURSOR_LIBRARIES}
    ${XI_LIBRARIES}
)

if(ENABLE_AUTH)
    target_link_libraries(nbody_simulation PRIVATE pqxx)
    target_compile_definitions(nbody_simulation PRIVATE ENABLE_AUTH)
endif()

if(NOT WIN32)
    target_link_libraries(nbody_simulation PRIVATE pthread)
endif()

# Install target
install(TARGETS nbody_simulation DESTINATION bin)
