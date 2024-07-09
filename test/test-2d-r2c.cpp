#include "fftw-cpp/fftw-cpp.h"
#include "util.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <numbers>
#include <vector>

TEST(Basic2dWrapperR2C, TwoWay) {
    namespace stdex = std::experimental;

    std::size_t N = 4, M = 6, NK = 3;
    using d2 = stdex::dextents<std::size_t, 2u>;
    fftw::basic_rmdbuffer<double, d2, std::complex<double>, stdex::layout_left> in{N, M}, out2{N, M};
    fftw::basic_mdbuffer<double, d2, std::complex<double>, stdex::layout_left> out{NK, M};

    auto p = fftw::plan_r2c<2u>::dft(in.to_mdspan(), out.to_mdspan(), fftw::Flags::ESTIMATE);
    auto pInv = fftw::plan_c2r<2u>::dft(out.to_mdspan(), out2.to_mdspan(), fftw::Flags::ESTIMATE);

    for (int j = 0; j < in.extent(0); ++j) {
        for (int k = 0; k < in.extent(1); ++k) {
            in(j, k) = std::cos(2.0 * std::numbers::pi * double((j + 1) * k) / (2.0 * double(in.size())));
        }
    }


    p();
    pInv();

    for (int j = 0; j < out2.extent(0); ++j) {
        for (int k = 0; k < out2.extent(1); ++k) {
            out2(j, k) /= double(out2.size());
        }
    }

    // TODO better matcher utils for mdspan
    std::span out2_span{out2.data(), out2.size()};
    std::span in_span{in.data(), in.size()};
    EXPECT_THAT(out2_span, ElementsAreComplexNear(in_span));
}