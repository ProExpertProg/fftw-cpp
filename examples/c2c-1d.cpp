#include <fftw-cpp/fftw-cpp.h>

#include <cassert>
#include <complex>
#include <fftw3.h>
#include <iostream>
#include <numbers>

void print(fftw::buffer const &buf) {
    for (int i = 0; i < buf.size(); ++i) {
        std::cout << buf[i] << " ";
    }
    std::cout << std::endl;
}

void run_fftw_cpp(int N) {
    fftw::buffer in(N), out(N), out2(N);

    auto p = fftw::plan<>::dft(in, out, fftw::FORWARD, fftw::Flags::ESTIMATE);
    auto pInv = fftw::plan<>::dft(out, out2, fftw::BACKWARD, fftw::Flags::ESTIMATE);

    for (int j = 0; j < N; ++j) {
        in[j] = {std::cos(2.0 * std::numbers::pi * j / N),
                 std::cos(2.0 * std::numbers::pi * (j + double(N) / 2) / N)};
    }

    p();
    pInv();

    print(in);
    print(out);
    print(out2);

    for (auto elem : out2) {
        std::cout << elem / double(N) << " ";
    }
}

void print(const fftw_complex *arr, int N) {
    for (int i = 0; i < N; ++i) {
        std::cout << "(" << arr[i][0] << "," << arr[i][1] << ") ";
    }
    std::cout << std::endl;
}

void run_fftw(int N) {
    fftw_complex *in, *out, *out2;
    fftw_plan p;

    in = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);
    out2 = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);
    assert(in != nullptr and out != nullptr and out2 != nullptr);

    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_plan pInv = fftw_plan_dft_1d(N, out, out2, FFTW_BACKWARD, FFTW_ESTIMATE);

    for (int j = 0; j < N; ++j) {
        in[j][0] = std::cos(2.0 * std::numbers::pi * j / N);
        in[j][1] = std::cos(2.0 * std::numbers::pi * (j + double(N) / 2) / N);
    }

    fftw_execute(p);
    fftw_execute(pInv);

    print(in, N);
    print(out, N);
    print(out2, N);

    auto *out2_cpp = reinterpret_cast<std::complex<double> *>(out2);
    for (int i = 0; i < N; ++i) {
        std::cout << out2_cpp[i] / double(N) << " ";
    }

    fftw_destroy_plan(p);
    fftw_destroy_plan(pInv);

    fftw_free(in);
    fftw_free(out);
    fftw_free(out2);
}

int main() {
    int N = 8;
    std::cout << "Running fftw-cpp" << std::endl;
    run_fftw_cpp(N);
    std::cout << std::endl;

    std::cout << "Running regular fftw" << std::endl;
    run_fftw(N);
}