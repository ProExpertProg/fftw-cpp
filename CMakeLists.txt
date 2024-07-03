cmake_minimum_required(VERSION 3.20) # TODO arbitrary, need to test, might need to drop later
project(fftw-cpp VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20) # TODO set to 23 if supported, but 20 is minimum for now. Can add support for more later

# Pull in mdspan
# TODO make optional (but required for tests)
include(FetchContent)
FetchContent_Declare(
        mdspan
        GIT_REPOSITORY https://github.com/kokkos/mdspan.git
        GIT_TAG 9ceface91483775a6c74d06ebf717bbb2768452f # TODO: update to latest (fix dextents and dynamic_extent)
)
FetchContent_MakeAvailable(mdspan)

add_library(fftw-cpp INTERFACE)
target_link_libraries(fftw-cpp INTERFACE fftw3 m mdspan)
target_include_directories(fftw-cpp INTERFACE include/)

option(FFTW_CPP_BUILD_TESTS "Build tests" ON)
if (FFTW_CPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()

option(FFTW_CPP_BUILD_EXAMPLES "Build examples" ON)
if (FFTW_CPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()
