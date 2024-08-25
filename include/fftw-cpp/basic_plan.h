#pragma once

#include "basic_buffer.h"
#include "util.h"
#include <memory>
#include <type_traits>

namespace fftw {

template <size_t D, class Real, class Complex> class plan_base {
  protected:
    using plan_t = detail::fftw_plan_t<Real>;
    std::unique_ptr<std::remove_pointer_t<plan_t>, decltype(&fftw_destroy_plan)> plan;

  public:
    plan_base() noexcept : plan(nullptr, &fftw_destroy_plan) {}

    explicit plan_base(plan_t ptr) noexcept : plan(ptr, &fftw_destroy_plan) {}

    /// Returns the underlying FFTW plan.
    plan_t c_plan() const { return plan.get(); }

    /// Executes the plan with the buffers provided initially.
    void operator()() const { fftw_execute(c_plan()); };
};

/// This boolean checks that the buffer is appropriate for this type of plan.
/// By default, it is false
template <size_t D, class Real, class Complex, typename T>
constexpr inline bool appropriate_buffer = false;

// We allow basic_buffer for 1D transforms
template <class Real, class Complex>
constexpr inline bool appropriate_buffer<1u, Real, Complex, basic_buffer<Real, Complex>> = true;

// We always allow a multi-d buffer for the same number of dimensions
template <size_t D, class Real, class Complex, typename Layout, typename ExtentsIndexType,
          ExtentsIndexType... I>
constexpr inline bool appropriate_buffer<
    D, Real, Complex,
    basic_mdbuffer<Real, MDSPAN::extents<ExtentsIndexType, I...>, Complex, Layout>> =
    sizeof...(I) == D;

template <size_t D, class Real, class Complex, typename T, typename T2>
concept appropriate_buffers =
    appropriate_buffer<D, Real, Complex, T> && appropriate_buffer<D, Real, Complex, T2>;

/// This boolean checks that the buffer is appropriate for this type of plan.
/// By default, it is false
template <size_t D, class Real, class Complex, typename T>
constexpr inline bool appropriate_view = false;

// We also allow a multi-d view for the same number of dimensions
template <size_t D, class Real, class Complex, typename Layout, typename ExtentsIndexType,
          ExtentsIndexType... I>
constexpr inline bool
    appropriate_view<D, Real, Complex,
                     MDSPAN::mdspan<Complex, MDSPAN::extents<ExtentsIndexType, I...>, Layout,
                                    MDSPAN::default_accessor<Complex>>> = sizeof...(I) == D;

template <size_t D, class Real, class Complex, typename T, typename T2>
concept appropriate_views =
    appropriate_view<D, Real, Complex, T> && appropriate_view<D, Real, Complex, T2>;

namespace detail {

template <bool IsReal, class Real, class Complex>
auto unwrap(basic_buffer<Real, Complex, IsReal> &buf) {
    return buf.unwrap();
}

// TODO this is just a fix until we get a proper mdbuffer
template <bool IsReal, typename Real, typename Complex, typename Extents, typename Layout>
auto unwrap(basic_mdbuffer<Real, Extents, Complex, Layout, IsReal> &buf) {
    // for now, this is what FFT expects
    // TODO proper underlying element type
    return reinterpret_cast<
        typename basic_buffer<Real, Complex, IsReal>::underlying_element_type *>(buf.data());
}

template <bool IsReal, typename Real, typename Complex, typename Extents, typename Layout>
auto *unwrap(MDSPAN::mdspan<std::conditional_t<IsReal, Real, Complex>, Extents, Layout,
                            MDSPAN::default_accessor<std::conditional_t<IsReal, Real, Complex>>>
                 view) {
    // for now, this is what FFT expects
    // TODO proper underlying element type
    return reinterpret_cast<underlying_element_type<IsReal, Real, Complex> *>(view.data_handle());
}

template <size_t D> std::array<fftw_iodim, D> get_dims(auto const &in, auto const &out) {
    using layout_in = typename std::decay_t<decltype(in)>::layout_type;
    using layout_out = typename std::decay_t<decltype(out)>::layout_type;

    constexpr bool in_right = std::same_as<MDSPAN::layout_right, layout_in>;
    constexpr bool out_right = std::same_as<MDSPAN::layout_right, layout_out>;

    std::array<fftw_iodim, D> dims;

    auto get_index = [](std::size_t i, bool is_right) -> std::size_t {
        return is_right ? i : (D - 1 - i);
    };

    for (int i = 0; i < D; i++) {
        dims[i].n =
            std::max(in.extent(get_index(i, in_right)), out.extent(get_index(i, out_right)));
        dims[i].is = in.stride(get_index(i, in_right));
        dims[i].os = out.stride(get_index(i, out_right));
    }

    return dims;
}

template <size_t D, class Real, class Complex>
    requires(D == 1u) && std::same_as<Real, double>
auto plan_dft_buffer(auto &in, auto &out, Direction direction, Flags flags) {
    return fftw_plan_dft_1d(in.size(), unwrap<false, Real, Complex>(in),
                            unwrap<false, Real, Complex>(out), static_cast<int>(direction), flags);
}

template <size_t D, class Real, class Complex>
    requires(D == 2u) && std::same_as<Real, double>
auto plan_dft_buffer(auto &in, auto &out, Direction direction, Flags flags) {
    return fftw_plan_dft_2d(in.extent(0), in.extent(1), unwrap<false, Real, Complex>(in),
                            unwrap<false, Real, Complex>(out), direction, flags);
}

template <size_t D, class Real, class Complex>
    requires std::same_as<Real, double>
auto plan_dft(auto &in, auto &out, Direction direction, Flags flags) {
    return fftw_plan_guru_dft(D, get_dims<D>(in, out).data(), 0, NULL,
                              unwrap<false, Real, Complex>(in), unwrap<false, Real, Complex>(out),
                              direction, flags);
}
} // namespace detail

template <size_t D, class Real, class Complex = std::complex<Real>>
class basic_plan : public plan_base<D, Real, Complex> {
  private:
    using base = plan_base<D, Real, Complex>;
    using plan_t = typename base::plan_t;

  public:
    using real_t = Real;
    using complex_t = Complex;

    using base::c_plan;
    using base::plan_base;
    using base::operator();

    template <typename BufferIn, typename BufferOut>
        requires appropriate_buffers<D, Real, Complex, BufferIn, BufferOut>
    basic_plan(BufferIn &in, BufferOut &out, Direction direction, Flags flags)
        : base(detail::template plan_dft_buffer<D, Real, Complex>(in, out, direction, flags)) {}

    template <typename ViewIn, typename ViewOut>
        requires appropriate_views<D, Real, Complex, ViewIn, ViewOut>
    basic_plan(ViewIn in, ViewOut out, Direction direction, Flags flags)
        : base(detail::template plan_dft<D, Real, Complex>(in, out, direction, flags)) {}

    template <typename BufferIn, typename BufferOut>
        requires appropriate_buffers<D, Real, Complex, BufferIn, BufferOut>
    void operator()(BufferIn &in, BufferOut &out) const;

    template <typename ViewIn, typename ViewOut>
        requires appropriate_views<D, Real, Complex, ViewIn, ViewOut>
    void operator()(ViewIn in, ViewOut out) const;
};

/// used for a static_assert inside an else block of if constexpr
template <class...> inline constexpr bool always_false = false;

template <size_t D, class Real, class Complex>
template <typename BufferIn, typename BufferOut>
    requires appropriate_buffers<D, Real, Complex, BufferIn, BufferOut>
void basic_plan<D, Real, Complex>::operator()(BufferIn &in, BufferOut &out) const {
    fftw_execute_dft(c_plan(), detail::unwrap<false, Real, Complex>(in),
                     detail::unwrap<false, Real, Complex>(out));
}

template <size_t D, class Real, class Complex>
template <typename ViewIn, typename ViewOut>
    requires appropriate_views<D, Real, Complex, ViewIn, ViewOut>
void basic_plan<D, Real, Complex>::operator()(ViewIn in, ViewOut out) const {
    fftw_execute_dft(c_plan(), detail::unwrap<false, Real, Complex>(in),
                     detail::unwrap<false, Real, Complex>(out));
}

namespace detail {
// TODO currently we support one layout left and one layout right,
//  but that actually changes which dimension is which.
//  E. g. if input is 4x6 col-major and output is 6x4 row-major, now
//  the dimensions in the output are reversed, kind of defeating the purpose of layouts
//  If people really want to switch layouts they'll have to reverse it themselves.

template <size_t... I> auto extents_impl(auto src, std::index_sequence<I...> indices) {
    return std::array<int, sizeof...(I)>{int(src.extent(I))...};
}

template <size_t D> std::array<int, D> extents(auto src) {
    return extents_impl(src, std::make_index_sequence<D>());
}

template <size_t D, class Real, class Complex>
    requires std::same_as<Real, double>
auto plan_dft_r2c(auto in, auto out, Flags flags) {
    return fftw_plan_guru_dft_r2c(D, get_dims<D>(in, out).data(), 0, NULL,
                                  unwrap<true, Real, Complex>(in),
                                  unwrap<false, Real, Complex>(out), flags);
}

template <size_t D, class Real, class Complex>
    requires std::same_as<Real, double>
auto plan_dft_c2r(auto &in, auto &out, Flags flags) {
    return fftw_plan_guru_dft_c2r(D, get_dims<D>(in, out).data(), 0, NULL,
                                  unwrap<false, Real, Complex>(in),
                                  unwrap<true, Real, Complex>(out), flags);
}
} // namespace detail

template <size_t D, class Real, class Complex = std::complex<Real>>
class basic_plan_r2c : public plan_base<D, Real, Complex> {
  private:
    using base = plan_base<D, Real, Complex>;
    using plan_t = typename base::plan_t;

  public:
    using real_t = Real;
    using complex_t = Complex;

    using base::c_plan;
    using base::plan_base;
    using base::operator();

    template <typename ViewIn, typename ViewOut>
    basic_plan_r2c(ViewIn in, ViewOut out, Flags flags)
        : base(detail::template plan_dft_r2c<D, Real, Complex>(in, out, flags)) {}

    template <typename ViewIn, typename ViewOut>
        requires appropriate_views<D, Real, Complex, ViewIn, ViewOut>
    void operator()(ViewIn in, ViewOut out) const;
};

template <size_t D, class Real, class Complex = std::complex<Real>>
class basic_plan_c2r : public plan_base<D, Real, Complex> {
  private:
    using base = plan_base<D, Real, Complex>;
    using plan_t = typename base::plan_t;

  public:
    using real_t = Real;
    using complex_t = Complex;

    using base::c_plan;
    using base::plan_base;
    using base::operator();

    template <typename ViewIn, typename ViewOut>
    basic_plan_c2r(ViewIn in, ViewOut out, Flags flags)
        : base(detail::template plan_dft_c2r<D, Real, Complex>(in, out, flags)) {}

    template <typename ViewIn, typename ViewOut>
        requires appropriate_views<D, Real, Complex, ViewIn, ViewOut>
    void operator()(ViewIn in, ViewOut out) const;
};

template <size_t D, class Real, class Complex>
template <typename ViewIn, typename ViewOut>
    requires appropriate_views<D, Real, Complex, ViewIn, ViewOut>
void basic_plan_r2c<D, Real, Complex>::operator()(ViewIn in, ViewOut out) const {
    fftw_execute_dft_r2c(c_plan(), detail::unwrap<true, Real, Complex>(in),
                         detail::unwrap<false, Real, Complex>(out));
}

template <size_t D, class Real, class Complex>
template <typename ViewIn, typename ViewOut>
    requires appropriate_views<D, Real, Complex, ViewIn, ViewOut>
void basic_plan_c2r<D, Real, Complex>::operator()(ViewIn in, ViewOut out) const {
    fftw_execute_dft_c2r(c_plan(), detail::unwrap<false, Real, Complex>(in),
                         detail::unwrap<true, Real, Complex>(out));
}

} // namespace fftw