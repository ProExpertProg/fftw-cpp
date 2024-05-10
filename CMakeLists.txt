cmake_minimum_required(VERSION 3.20) # TODO arbitrary, need to test, might need to drop later
project(fftw-cpp VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20) # TODO set to 23 if supported, but 20 is minimum for now. Can add support for more later

# Pull in mdspan
# TODO make optional
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

function(fftw_test NAME)
add_executable(${NAME} ${ARGN})
target_link_libraries(${NAME} fftw-cpp)
add_test(${NAME} ${NAME})
endfunction()

fftw_test(test-fftw test.cpp)
fftw_test(test-fftw-wrapper test-wrapper.cpp)
fftw_test(test-fftw-wrapper-2d test-wrapper-2d.cpp)
fftw_test(test-fftw-r2c-2d test-r2c-2d.cpp)
fftw_test(test-plan test-plan.cpp)
fftw_test(test-mdspan test-mdspan.cpp)
