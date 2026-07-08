cmake_minimum_required(VERSION 3.24)

project(InteractiveVideoCodecVisualizer
    VERSION 0.1.0
    DESCRIPTION "Interactive Video Codec Visualizer - see how every pixel becomes a compressed bitstream"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Keep compile_commands.json around for tooling (clangd, IDEs).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(IVCV_BUILD_TESTS "Build unit tests" ON)
option(IVCV_BUILD_APP "Build the Qt desktop application" ON)
option(IVCV_BUILD_BENCHMARKS "Build Google Benchmark targets" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(FetchContent)

# ---------------------------------------------------------------------------
# Third-party dependencies resolved via find_package (expected to be
# installed on the system / provided by the platform package manager).
# ---------------------------------------------------------------------------
find_package(Eigen3 3.4 REQUIRED NO_MODULE)
find_package(OpenCV 4 REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

if(IVCV_BUILD_APP)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
endif()

# ---------------------------------------------------------------------------
# Third-party dependencies resolved via FetchContent. Test/benchmark
# frameworks are pinned here so contributors do not need to install them
# system-wide.
# ---------------------------------------------------------------------------
if(IVCV_BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
endif()

if(IVCV_BUILD_BENCHMARKS)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.0
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)
endif()

add_subdirectory(core)

if(IVCV_BUILD_APP)
    add_subdirectory(app)
endif()

if(IVCV_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(IVCV_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
