#include "fftw-cpp/fftw-cpp.h"
#include "util.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <numbers>
#include <vector>

#include "helper.cpp"

TEST(Basic2dWrapperR2CLeft, TwoWay) {
    namespace stdex = std::experimental;

    std::size_t N = 4, M = 6, NK = 3;
    using d2 = stdex::dextents<std::size_t, 2u>;
    fftw::basic_rmdbuffer<double, d2, std::complex<double>, stdex::layout_left> in{N, M},
        out2{N, M};
    fftw::basic_mdbuffer<double, d2, std::complex<double>, stdex::layout_left> out{NK, M};

    auto p = fftw::plan_r2c<2u>(in.to_mdspan(), out.to_mdspan(), fftw::Flags::ESTIMATE);
    auto pInv = fftw::plan_c2r<2u>(out.to_mdspan(), out2.to_mdspan(), fftw::Flags::ESTIMATE);

    fill(in);

    p();
    pInv();

    normalise(out2);

    // TODO better matcher utils for mdspan
    std::span out2_span{out2.data(), out2.size()};
    std::span in_span{in.data(), in.size()};
    EXPECT_THAT(out2_span, ElementsAreComplexNear(in_span));
}

TEST(Basic2dWrapperR2CRight, TwoWay) {
    namespace stdex = std::experimental;

    std::size_t N = 4, M = 6, NK = 3;
    using d2 = stdex::dextents<std::size_t, 2u>;
    fftw::basic_rmdbuffer<double, d2, std::complex<double>, stdex::layout_right> in{M, N},
        out2{M, N};
    fftw::basic_mdbuffer<double, d2, std::complex<double>, stdex::layout_right> out{M, NK};

    auto p = fftw::plan_r2c<2u>(in.to_mdspan(), out.to_mdspan(), fftw::Flags::ESTIMATE);
    auto pInv = fftw::plan_c2r<2u>(out.to_mdspan(), out2.to_mdspan(), fftw::Flags::ESTIMATE);

    fill(in);

    p();
    pInv();

    normalise(out2);

    // TODO better matcher utils for mdspan
    std::span out2_span{out2.data(), out2.size()};
    std::span in_span{in.data(), in.size()};
    EXPECT_THAT(out2_span, ElementsAreComplexNear(in_span));
}

TEST(Basic2dWrapperR2CLFirst, TwoWay) {
    namespace stdex = std::experimental;

    std::size_t N = 4, M = 6, NK = 3;
    using d2 = stdex::dextents<std::size_t, 2u>;
    fftw::basic_rmdbuffer<double, d2, std::complex<double>, stdex::layout_left> in{N, M},
        out2{N, M};
    fftw::basic_mdbuffer<double, d2, std::complex<double>, stdex::layout_right> out{M, NK};

    auto p = fftw::plan_r2c<2u>(in.to_mdspan(), out.to_mdspan(), fftw::Flags::ESTIMATE);
    auto pInv = fftw::plan_c2r<2u>(out.to_mdspan(), out2.to_mdspan(), fftw::Flags::ESTIMATE);

    fill(in);

    p();
    pInv();

    normalise(out2);

    // TODO better matcher utils for mdspan
    std::span out2_span{out2.data(), out2.size()};
    std::span in_span{in.data(), in.size()};
    EXPECT_THAT(out2_span, ElementsAreComplexNear(in_span));
}

TEST(Basic2dWrapperR2CRFirst, TwoWay) {
    namespace stdex = std::experimental;

    std::size_t N = 4, M = 6, NK = 3;
    using d2 = stdex::dextents<std::size_t, 2u>;
    fftw::basic_rmdbuffer<double, d2, std::complex<double>, stdex::layout_right> in{M, N},
        out2{M, N};
    fftw::basic_mdbuffer<double, d2, std::complex<double>, stdex::layout_left> out{NK, M};

    auto p = fftw::plan_r2c<2u>(in.to_mdspan(), out.to_mdspan(), fftw::Flags::ESTIMATE);
    auto pInv = fftw::plan_c2r<2u>(out.to_mdspan(), out2.to_mdspan(), fftw::Flags::ESTIMATE);

    fill(in);

    p();
    pInv();

    normalise(out2);

    // TODO better matcher utils for mdspan
    std::span out2_span{out2.data(), out2.size()};
    std::span in_span{in.data(), in.size()};
    EXPECT_THAT(out2_span, ElementsAreComplexNear(in_span));
}