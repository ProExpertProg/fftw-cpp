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

TEST(basicBothWays, Sanity){
    namespace stdex = std::experimental;
    using d2 = stdex::dextents<std::size_t, 2u>;
    using complex = std::complex<double>;

    int dim = 4;
    double in_data[4][4] = {
        {1.0, 1.0, 1.0, 1.0},
        {1.0, 1.0, 1.0, 1.0},
        {1.0, 1.0, 1.0, 1.0},
        {1.0, 1.0, 1.0, 1.0}
    };

    fftw_complex out_data[dim][dim];

    fftw::basic_rmdbuffer<double, d2, std::complex<double>, stdex::layout_left> r_buff{dim, dim};
    fftw::basic_mdbuffer<double, d2, std::complex<double>> c_out{dim, dim};

    for(int i = 0; i < dim; i++) {
        for(int j = 0; j < dim; j++) {
            r_buff(i, j) = 1;   /// in_data[i][j];
        }
    }

    fftw_plan p = fftw_plan_dft_r2c_2d(dim, dim, (double*)in_data, (fftw_complex*)out_data, FFTW_ESTIMATE);
    auto p_cpp = fftw::plan_r2c<2u>(r_buff.to_mdspan(), c_out.to_mdspan(), fftw::Flags::ESTIMATE);

    fftw_execute(p);
    p_cpp();

    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            std::cout << "C:" << out_data[i][j][0] << " + " << out_data[i][j][1] << "i\n";
            std::cout << "Cpp:" << c_out.data()[i*dim + j].real() << " + " << c_out.data()[i*dim + j].imag() << "i\n";
            // EXPECT_THAT(out_data[i][j][0], c_out.data()[i*dim + j].real());
            // EXPECT_THAT(out_data[i][j][1], c_out.data()[i*dim + j].imag());
        }
        std::cout << std::endl;
    }
    EXPECT_THAT(out_data[0][3][0], c_out.data()[3].real());

    fftw_destroy_plan(p);

}
