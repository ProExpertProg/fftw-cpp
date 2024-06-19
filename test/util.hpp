#pragma once

#include <gmock/gmock.h>

constexpr double TOLERANCE = 1e-10;

MATCHER_P(IsComplexNear, val, "near " + ::testing::PrintToString(val)) {
    if (std::abs(arg - val) > TOLERANCE) {
        *result_listener << arg;
        return false;
    }

    return true;
}

MATCHER_P(ElementsAreComplexNear, buf_expected, "values near " + ::testing::PrintToString(buf_expected)) {
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
