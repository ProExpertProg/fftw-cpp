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
};

/// This concept checks that the layout is appropriate for this type of plan.
/// By default, it is false
template <typename Layout>
concept appropriate_layout = std::same_as<MDSPAN::layout_right, Layout>;
// TODO support for other layouts

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
template <size_t D, class Real, class Complex, appropriate_layout Layout, typename ExtentsIndexType,
          ExtentsIndexType... I>
constexpr inline bool
    appropriate_view<D, Real, Complex,
                     MDSPAN::mdspan<Complex, MDSPAN::extents<ExtentsIndexType, I...>, Layout,
                                    MDSPAN::default_accessor<Complex>>> = sizeof...(I) == D;

template <size_t D, class Real, class Complex, typename T, typename T2>
concept appropriate_views =
    appropriate_view<D, Real, Complex, T> && appropriate_view<D, Real, Complex, T2>;

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

    /// Executes the plan with the buffers provided initially.
    void operator()() const;

    template <typename BufferIn, typename BufferOut>
        requires appropriate_buffers<D, Real, Complex, BufferIn, BufferOut>
    void operator()(BufferIn &in, BufferOut &out) const;

    template <typename ViewIn, typename ViewOut>
        requires appropriate_views<D, Real, Complex, ViewIn, ViewOut>
    void operator()(ViewIn in, ViewOut out) const;

    /// \defgroup{planning utilities}
    template <typename BufferIn, typename BufferOut>
        requires appropriate_buffers<D, Real, Complex, BufferIn, BufferOut>
    static auto dft(BufferIn &in, BufferOut &out, Direction direction, Flags flags) -> basic_plan;

    template <typename ViewIn, typename ViewOut>
        requires appropriate_views<D, Real, Complex, ViewIn, ViewOut>
    static auto dft(ViewIn in, ViewOut out, Direction direction, Flags flags) -> basic_plan;
};

template <size_t D, class Real, class Complex>
void basic_plan<D, Real, Complex>::operator()() const {
    fftw_execute(c_plan());
}

/// used for a static_assert inside an else block of if constexpr
template <class...> inline constexpr bool always_false = false;

namespace detail {

template <bool IsReal, class Real, class Complex>
auto unwrap(basic_buffer<Real, Complex, IsReal> &buf) {
    return buf.unwrap();
}

// TODO this is just a fix until we get a proper mdbuffer
template <bool IsReal, typename Real, typename Complex, typename Extents, appropriate_layout Layout>
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

template <size_t D, class Real, class Complex>
    requires(D == 1u) && std::same_as<Real, double>

auto plan_dft(auto &in, auto &out, Direction direction, Flags flags) {
    return fftw_plan_dft_1d(in.size(), unwrap<false, Real, Complex>(in),
                            unwrap<false, Real, Complex>(out), direction, flags);
}

template <size_t D, class Real, class Complex>
    requires(D == 2u) && std::same_as<Real, double>

auto plan_dft(auto &in, auto &out, Direction direction, Flags flags) {
    // TODO for layout left this is different
    return fftw_plan_dft_2d(in.extent(0), in.extent(1), unwrap<false, Real, Complex>(in),
                            unwrap<false, Real, Complex>(out), direction, flags);
}
} // namespace detail

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

template <size_t D, class Real, class Complex>
template <typename BufferIn, typename BufferOut>
    requires appropriate_buffers<D, Real, Complex, BufferIn, BufferOut>
auto basic_plan<D, Real, Complex>::dft(BufferIn &in, BufferOut &out, Direction direction,
                                       Flags flags) -> basic_plan {
    if (in.size() != out.size()) { throw std::invalid_argument("mismatched buffer sizes"); }
    if (direction != FORWARD and direction != BACKWARD) {
        throw std::invalid_argument("invalid direction");
    }

    auto c_plan = detail::template plan_dft<D, Real, Complex>(in, out, direction, flags);
    return basic_plan{c_plan};
}

// TODO dedup
template <size_t D, class Real, class Complex>
template <typename ViewIn, typename ViewOut>
    requires appropriate_views<D, Real, Complex, ViewIn, ViewOut>
auto basic_plan<D, Real, Complex>::dft(ViewIn in, ViewOut out, Direction direction, Flags flags)
    -> basic_plan {
    if (in.size() != out.size()) { throw std::invalid_argument("mismatched buffer sizes"); }
    if (direction != FORWARD and direction != BACKWARD) {
        throw std::invalid_argument("invalid direction");
    }

    auto c_plan = detail::template plan_dft<D, Real, Complex>(in, out, direction, flags);
    return basic_plan{c_plan};
}

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

    /// Executes the plan with the buffers provided initially.
    void operator()() const;

    template <typename ViewIn, typename ViewOut> void operator()(ViewIn in, ViewOut out) const;

    /// \defgroup{planning utilities}
    template <typename ViewIn, typename ViewOut>
    static auto dft(ViewIn in, ViewOut out, Flags flags) -> basic_plan_r2c;
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

    /// Executes the plan with the buffers provided initially.
    void operator()() const;

    template <typename ViewIn, typename ViewOut> void operator()(ViewIn in, ViewOut out) const;

    /// \defgroup{planning utilities}
    template <typename ViewIn, typename ViewOut>
    static auto dft(ViewIn in, ViewOut out, Flags flags) -> basic_plan_c2r;
};

template <size_t D, class Real, class Complex>
void basic_plan_r2c<D, Real, Complex>::operator()() const {
    fftw_execute(c_plan());
}

template <size_t D, class Real, class Complex>
template <typename ViewIn, typename ViewOut>
void basic_plan_r2c<D, Real, Complex>::operator()(ViewIn in, ViewOut out) const {
    fftw_execute_dft_r2c(c_plan(), detail::unwrap<true, Real, Complex>(in),
                         detail::unwrap<false, Real, Complex>(out));
}

template <size_t D, class Real, class Complex>
void basic_plan_c2r<D, Real, Complex>::operator()() const {
    fftw_execute(c_plan());
}

template <size_t D, class Real, class Complex>
template <typename ViewIn, typename ViewOut>
void basic_plan_c2r<D, Real, Complex>::operator()(ViewIn in, ViewOut out) const {
    fftw_execute_dft_c2r(c_plan(), detail::unwrap<false, Real, Complex>(in),
                         detail::unwrap<true, Real, Complex>(out));
}

namespace detail {
// TODO currently we support one layout left and one layout right,
//  but that actually changes which dimension is which.
//  E. g. if input is 4x6 col-major and output is 6x4 row-major, now
//  the dimensions in the output are reversed, kind of defeating the purpose of layouts
//  If people really want to switch layouts they'll have to reverse it themselves.

template <size_t D> std::array<int, D> dims(auto in, auto out) {
    static_assert(D == 2u && "Currently only supporting 2D");
    using layout_in = typename std::decay_t<decltype(in)>::layout_type;
    using layout_out = typename std::decay_t<decltype(out)>::layout_type;

    // TODO layouts other than right and left
    constexpr bool in_right = std::same_as<MDSPAN::layout_right, layout_in>;
    constexpr bool out_right = std::same_as<MDSPAN::layout_right, layout_out>;

    auto Validate = [&](bool condition) {
        if (!condition) { throw std::invalid_argument("Extents don't match"); }
    };

    if (in_right == out_right) {
        Validate(in.extent(0) == out.extent(0));
        Validate(in.extent(1) == out.extent(1));
    } else {
        Validate(in.extent(0) == out.extent(1));
        Validate(in.extent(1) == out.extent(0));
    }

    if constexpr (in_right) {
        return {int(in.extent(0)), int(in.extent(1))};
    } else {
        return {int(in.extent(1)), int(in.extent(0))};
    }
}

template <size_t... I> auto extents_impl(auto src, std::index_sequence<I...> indices) {
    return std::array<int, sizeof...(I)>{int(src.extent(I))...};
}

template <size_t D> std::array<int, D> extents(auto src) {
    return extents_impl(src, std::make_index_sequence<D>());
}

template <size_t D> std::array<int, D> dims_r2c(auto r, auto c) {
    static_assert(D == 2u && "Currently only supporting 2D");
    using layout_r = typename std::decay_t<decltype(r)>::layout_type;
    using layout_c = typename std::decay_t<decltype(c)>::layout_type;

    // TODO layouts other than right and left
    constexpr bool r_right = std::same_as<MDSPAN::layout_right, layout_r>;
    constexpr bool c_right = std::same_as<MDSPAN::layout_right, layout_c>;

    auto Validate = [&](bool condition) {
        if (!condition) { throw std::invalid_argument("Extents don't match"); }
    };

    std::array<int, D> r_extents{extents<D>(r)}, c_extents{extents<D>(c)};
    auto Swap = [&](std::array<int, D> &a) { std::swap(a[0], a[1]); };

    if (!r_right) { Swap(r_extents); }
    if (!c_right) { Swap(c_extents); }
    // extent

    Validate(r_extents[0] == c_extents[0]);
    Validate(r_extents[1] / 2 + 1 == c_extents[1]);

    return r_extents;
}

template <size_t D, class Real, class Complex>
    requires std::same_as<Real, double>
auto plan_dft_r2c(auto in, auto out, Flags flags) {
    return fftw_plan_dft_r2c(D, dims_r2c<D>(in, out).data(), unwrap<true, Real, Complex>(in),
                             unwrap<false, Real, Complex>(out), flags);
}

template <size_t D, class Real, class Complex>
    requires std::same_as<Real, double>
auto plan_dft_c2r(auto in, auto out, Flags flags) {
    return fftw_plan_dft_c2r(D, dims_r2c<D>(out, in).data(), unwrap<false, Real, Complex>(in),
                             unwrap<true, Real, Complex>(out), flags);
}
} // namespace detail

template <size_t D, class Real, class Complex>
template <typename ViewIn, typename ViewOut>
auto basic_plan_r2c<D, Real, Complex>::dft(ViewIn in, ViewOut out, fftw::Flags flags)
    -> basic_plan_r2c<D, Real, Complex> {
    auto c_plan = detail::template plan_dft_r2c<D, Real, Complex>(in, out, flags);
    return basic_plan_r2c{c_plan};
}

template <size_t D, class Real, class Complex>
template <typename ViewIn, typename ViewOut>
auto basic_plan_c2r<D, Real, Complex>::dft(ViewIn in, ViewOut out, fftw::Flags flags)
    -> basic_plan_c2r<D, Real, Complex> {
    auto c_plan = detail::template plan_dft_c2r<D, Real, Complex>(in, out, flags);
    return basic_plan_c2r{c_plan};
}

} // namespace fftw