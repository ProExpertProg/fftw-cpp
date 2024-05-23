include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG 9b4993ca7d1279dec5c5d41ba327cb11a77bdc00
)
FetchContent_MakeAvailable(googletest)

add_executable(fftw-cpp-tests
        run-all-tests.cpp
        test-1d-c2c.cpp
)
target_link_libraries(fftw-cpp-tests fftw-cpp gmock_main)
add_test(fftw-cpp-all-tests fftw-cpp-tests)
