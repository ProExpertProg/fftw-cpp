#include <complex>

void fill(auto &in) {

    for (int j = 0; j < in.extent(0); ++j) {
        for (int k = 0; k < in.extent(1); ++k) {
            in(j, k) =
                std::cos(2.0 * std::numbers::pi * double((j + 1) * k) / (2.0 * double(in.size())));
        }
    }
}

void normalise(auto &in) {
    for (int j = 0; j < in.extent(0); ++j) {
        for (int k = 0; k < in.extent(1); ++k) {
            in(j, k) /= double(in.size());
        }
    }
}