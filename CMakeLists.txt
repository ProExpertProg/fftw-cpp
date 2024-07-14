cmake_minimum_required(VERSION 3.20) # TODO arbitrary, need to test, might need to drop later
project(fftw-cpp VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20) # TODO set to 23 if supported, but 20 is minimum for now. Can add support for more later

# Pull in mdspan
# TODO make optional (but required for tests)
include(FetchContent)
include(ExternalProject)
FetchContent_Declare(
        mdspan
        GIT_REPOSITORY https://github.com/kokkos/mdspan.git
        GIT_TAG 9ceface91483775a6c74d06ebf717bbb2768452f # TODO: update to latest (fix dextents and dynamic_extent)
)
FetchContent_MakeAvailable(mdspan)

# Find fftw3
#find_package(FFTW3)
#if (NOT FFTW3_FOUND)
#    message(WARNING "FFTW3 not found, make sure fftw3 is available globally")
#endif ()

option(FFTW_CPP_USE_MKL "Use MKL for FFTW3" OFF) # TODO: add support for other FFTW3 backends
if (FFTW_CPP_USE_MKL)
    message(FATAL_ERROR "MKL backend not supported yet")
endif ()

option(FFTW_CPP_DOWNLOAD_FFTW3 "Download FFTW3" OFF)

if (FFTW_CPP_DOWNLOAD_FFTW3)
    set(FFTW3_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/fftw3-install)
#    file(MAKE_DIRECTORY ${FFTW3_INSTALL_PREFIX})

    set(FFTW3_CONFIGURE_ARGS "")


    option(FFTW_CPP_FFTW3_AVX512 ON)
    if (FFTW_CPP_FFTW3_AVX512)
        set(FFTW3_CONFIGURE_ARGS "${FFTW3_CONFIGURE_ARGS} --enable-avx512")
    endif()

    # TODO patch for CMake find_package?
    ExternalProject_Add(
            fftw3
            URL https://www.fftw.org/fftw-3.3.10.tar.gz
            URL_MD5 8ccbf6a5ea78a16dbc3e1306e234cc5c
            CONFIGURE_COMMAND ./configure --prefix=${FFTW3_INSTALL_PREFIX} ${FFTW3_CONFIGURE_ARGS} CC=${CMAKE_C_COMPILER}
            # build and install use make by default
            BUILD_IN_SOURCE ON # required for configure to work
    )

    # TODO set vars that tell the module FindFFTW3 where the library is
endif ()

# TODO move into FindFFTW3.cmake
find_path(FFTW3_INCLUDE_DIRS fftw3.h PATH_SUFFIXES fftw3)
find_library(FFTW3_LIBRARIES NAMES fftw3)

add_library(fftw-cpp INTERFACE)
target_link_libraries(fftw-cpp INTERFACE ${FFTW3_LIBRARIES} m mdspan)
target_include_directories(fftw-cpp INTERFACE include/ ${FFTW3_INCLUDE_DIRS})

option(FFTW_CPP_BUILD_TESTS "Build tests" ON)
if (FFTW_CPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()

option(FFTW_CPP_BUILD_EXAMPLES "Build examples" ON)
if (FFTW_CPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()
