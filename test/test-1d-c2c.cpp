#include "fftw-cpp/fftw-cpp.h"
#include "util.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <numbers>
#include <vector>

TEST(Basic1dWrapper, TwoWay) {
    int N = 8;

    fftw::buffer in(N), out(N), out2(N);

    auto p = fftw::plan<>::dft(in, out, fftw::FORWARD, fftw::Flags::ESTIMATE);
    auto pInv = fftw::plan<>::dft(out, out2, fftw::BACKWARD, fftw::Flags::ESTIMATE);

    for (int j = 0; j < N; ++j) {
        in[j] = {
                std::cos(2.0 * std::numbers::pi * j / N),
                std::cos(2.0 * std::numbers::pi * (j + double(N) / 2) / N)
        };
    }

    p();
    pInv();

    for (auto &elem: out2) {
        elem /= double(N);
    }

    EXPECT_THAT(out2, ElementsAreComplexNear(in));
}