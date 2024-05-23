#include "fftw-cpp/fftw-cpp.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <numbers>
#include <vector>

using ::testing::ExplainMatchResult;
using ::testing::PrintToString;

constexpr double TOLERANCE = 1e-10;

MATCHER_P(IsComplexNear, val, "near " + PrintToString(val)) {
    if (std::abs(arg - val) > TOLERANCE) {
        *result_listener << arg;
        return false;
    }

    return true;
}

MATCHER_P(ElementsAreComplexNear, buf_expected, "values near " + PrintToString(buf_expected)) {
    if (arg.size() != buf_expected.size()) {
        *result_listener << "Vectors have different sizes";
        return false;
    }

    for (int i = 0; i < arg.size(); ++i) {
        if (!ExplainMatchResult(IsComplexNear(buf_expected[i]), arg[i], result_listener)) {
            *result_listener << " at index " << i;
            return false;
        }
    }

    return true;
}

auto ElementsAreComplexNear(const fftw::buffer &buf_expected) {
    std::vector<std::complex<double>> buf_expected_data(buf_expected.size());
    std::ranges::copy(buf_expected, buf_expected_data.begin());

    return ElementsAreComplexNear(std::move(buf_expected_data));
}

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