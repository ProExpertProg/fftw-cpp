include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG 9b4993ca7d1279dec5c5d41ba327cb11a77bdc00
)
FetchContent_MakeAvailable(googletest)

function(add_fftw_test_impl NAME)
    add_executable(${NAME} ${ARGN})
    target_link_libraries(${NAME} fftw-cpp GTest::gmock_main)
    add_test(${NAME} ${NAME})
endfunction()

function(add_fftw_test FILE)
    string(REGEX REPLACE ".cpp$" "" NAME ${FILE})
    add_fftw_test_impl(${NAME} ${FILE} ${ARGN})
endfunction()

add_fftw_test(test-1d-c2c.cpp)
add_fftw_test(test-2d-r2c.cpp)
