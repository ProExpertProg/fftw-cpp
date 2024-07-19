# Options (default value in parenthesis):
# - FFTW_CPP_BUILD_TESTS   (OFF): Build unit tests
# - FFTW_CPP_BUILD_EXAMPLES (ON): Build examples
# - FFTW_CPP_MDSPAN         (ON): Pull in mdspan dependency
# - FFTW_CPP_USE_MKL       (OFF): Use MKL for FFTW3 (not supported yet)
# Options controlling FFT3 download
# - FFTW_CPP_FFTW3_DOWNLOAD       (OFF): Download FFTW3 during the build step and link to it instead of using an installed version
# - FFTW_CPP_FFTW3_AVX512          (ON): Enable AVX512 for FFTW3 (if downloaded)
# - FFTW_CPP_FFTW3_CONFIGURE_FLAGS (""): Extra flags to pass to downloaded FFTW3 configure script

cmake_minimum_required(VERSION 3.20) # TODO arbitrary, need to test, might need to drop later
project(fftw-cpp VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20) # TODO set to 23 if supported, but 20 is minimum for now. Can add support for more later

# Options
option(FFTW_CPP_BUILD_TESTS "Build tests" ON)
option(FFTW_CPP_BUILD_EXAMPLES "Build examples" ON)
# if this becomes OFF by default, make sure mdspan gets included when tests or examples are enabled.
option(FFTW_CPP_MDSPAN "Pull in mdspan dependency" ON)
option(FFTW_CPP_USE_MKL "Use MKL for FFTW3" OFF) # TODO: add support for other FFTW3 backends

option(FFTW_CPP_DOWNLOAD_FFTW3 "Download FFTW3 during the build step and link to it instead of using an installed version" OFF)
option(FFTW_CPP_FFTW3_AVX512 ON)
set(FFTW_CPP_FFTW3_CONFIGURE_FLAGS "" CACHE STRING "Extra flags to pass to downloaded FFTW3 configure script")

# Add cmake directory to module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Add fftw-cpp library
add_library(fftw-cpp INTERFACE)

# DEPENDENCIES
include(FetchContent)
include(ExternalProject)
FetchContent_Declare(
        mdspan
        GIT_REPOSITORY https://github.com/kokkos/mdspan.git
        GIT_TAG 9ceface91483775a6c74d06ebf717bbb2768452f # TODO: update to latest (fix dextents and dynamic_extent)
)

if (FFTW_CPP_MDSPAN)
    FetchContent_MakeAvailable(mdspan)
    target_link_libraries(fftw-cpp INTERFACE mdspan)
endif ()

if (FFTW_CPP_USE_MKL)
    message(FATAL_ERROR "MKL backend not supported yet")
endif ()

if (FFTW_CPP_DOWNLOAD_FFTW3)
    message(STATUS "Downloading FFTW3")
    set(_FFTW3_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/fftw3-install)
    set(_FFTW3_CONFIGURE_ARGS ${FFTW_CPP_FFTW3_CONFIGURE_FLAGS})

    if (FFTW_CPP_FFTW3_AVX512)
        set(_FFTW3_CONFIGURE_ARGS "${_FFTW3_CONFIGURE_ARGS} --enable-avx512")
    endif()

    # This happens at build time, so we can't use find_package
    ExternalProject_Add(
            fftw3-download
            URL https://www.fftw.org/fftw-3.3.10.tar.gz
            URL_MD5 8ccbf6a5ea78a16dbc3e1306e234cc5c
            CONFIGURE_COMMAND ./configure --prefix=${_FFTW3_INSTALL_PREFIX} ${_FFTW3_CONFIGURE_ARGS} CC=${CMAKE_C_COMPILER}
            BUILD_IN_SOURCE ON # required for configure to work
    )

    # Do not link directly, as the library file does not exist pre-download and
    # so the build system (e.g. ninja) does not know the dependency.
    target_include_directories(fftw-cpp INTERFACE ${_FFTW3_INSTALL_PREFIX}/include)
    target_link_directories(fftw-cpp INTERFACE ${_FFTW3_INSTALL_PREFIX}/lib/)
    target_link_libraries(fftw-cpp INTERFACE fftw3)
    add_dependencies(fftw-cpp fftw3-download)

    if (UNIX)
        target_link_libraries(fftw-cpp INTERFACE m)
    endif ()
else ()
    # Config mode is broken for autotools-build fftw version<=3.3.10
    find_package(FFTW3 MODULE REQUIRED)
    target_link_libraries(fftw-cpp INTERFACE FFTW3::fftw3)
endif ()

target_include_directories(fftw-cpp INTERFACE include/)

if (FFTW_CPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()

if (FFTW_CPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()
