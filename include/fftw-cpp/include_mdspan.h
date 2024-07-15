#pragma once

// =================
// Check mdspan support
// =================

#ifdef FFTW_CPP_MDSPAN_NAMESPACE
namespace fftw {
/// Namespace that contains mdspan/mdarray and helpers (extents, default_accessor, etc.).
/// Given by user.
namespace MDSPAN = FFTW_CPP_MDSPAN_NAMESPACE;
}; // namespace fftw
#elif __cpp_lib_mdspan >= 202207L
#include <mdarray>
#include <mdspan>

namespace fftw {
/// Namespace that contains mdspan/mdarray and helpers (extents, default_accessor, etc.)
namespace MDSPAN = ::std;
}; // namespace fftw
#elif __has_include(<experimental/mdspan>)

#include <experimental/mdarray>
#include <experimental/mdspan>

namespace fftw {
/// Namespace that contains mdspan/mdarray and helpers (extents, default_accessor, etc.)
namespace MDSPAN = ::std::experimental;
}; // namespace fftw
#else
#error mdspan support required
#endif