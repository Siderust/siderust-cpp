#pragma once

/**
 * @file centers.hpp
 * @brief Zero-sized reference-center tag types mirroring Rust's centers module.
 *
 * Each center is a unique empty struct used as a compile-time tag for
 * position templates.  Every tag carries a static `ffi_id` mapping to
 * the C enum, and optionally a Params type for parameterized centers
 * (Topocentric, Bodycentric).
 */

#include "ffi_core.hpp"

namespace siderust {

// Forward declaration for Geodetic (defined in coordinates/geodetic.hpp)
struct Geodetic;

namespace centers {

// ============================================================================
// Center Trait
// ============================================================================

template <typename C> struct CenterTraits; // primary — intentionally undefined

template <typename C, typename = void> struct is_center : std::false_type {};

template <typename C>
struct is_center<C, std::void_t<decltype(CenterTraits<C>::ffi_id)>>
    : std::true_type {};

template <typename C> inline constexpr bool is_center_v = is_center<C>::value;

// ============================================================================
// Center Tag Definitions
// ============================================================================

/// Solar-system barycenter (zero-cost, Params = void).
struct Barycentric {};
/// Heliocenter (zero-cost, Params = void).
struct Heliocentric {};
/// Geocenter (zero-cost, Params = void).
struct Geocentric {};
/// Observer on Earth's surface (Params = Geodetic).
struct Topocentric {};
/// Center of a body (Params = BodycentricParams).
struct Bodycentric {};

// ============================================================================
// CenterTraits Specializations
// ============================================================================

/// Marker for simple (no-parameter) centers.
struct NoParams {};

template <> struct CenterTraits<Barycentric> {
  static constexpr siderust_center_t ffi_id = SIDERUST_CENTER_T_BARYCENTRIC;
  using Params = NoParams;
  static constexpr const char *name() { return "Barycentric"; }
};

template <> struct CenterTraits<Heliocentric> {
  static constexpr siderust_center_t ffi_id = SIDERUST_CENTER_T_HELIOCENTRIC;
  using Params = NoParams;
  static constexpr const char *name() { return "Heliocentric"; }
};

template <> struct CenterTraits<Geocentric> {
  static constexpr siderust_center_t ffi_id = SIDERUST_CENTER_T_GEOCENTRIC;
  using Params = NoParams;
  static constexpr const char *name() { return "Geocentric"; }
};

template <> struct CenterTraits<Topocentric> {
  static constexpr siderust_center_t ffi_id = SIDERUST_CENTER_T_TOPOCENTRIC;
  using Params = Geodetic; // forward-declared
  static constexpr const char *name() { return "Topocentric"; }
};

template <> struct CenterTraits<Bodycentric> {
  static constexpr siderust_center_t ffi_id = SIDERUST_CENTER_T_BODYCENTRIC;
  using Params = NoParams; // placeholder for BodycentricParams
  static constexpr const char *name() { return "Bodycentric"; }
};

// ============================================================================
// Center-shift Valid Predicate
// ============================================================================

/**
 * @brief Marks center pairs for which a CenterShiftProvider exists.
 *
 * Hub = Barycentric.  Supported shifts: Barycentric ↔ Heliocentric,
 * Barycentric ↔ Geocentric, and the composed Heliocentric ↔ Geocentric.
 */
template <typename From, typename To>
struct has_center_transform : std::false_type {};

template <typename C> struct has_center_transform<C, C> : std::true_type {};

#define SIDERUST_CENTER_TRANSFORM_PAIR(A, B)                                   \
  template <> struct has_center_transform<A, B> : std::true_type {};           \
  template <> struct has_center_transform<B, A> : std::true_type {}

SIDERUST_CENTER_TRANSFORM_PAIR(Barycentric, Heliocentric);
SIDERUST_CENTER_TRANSFORM_PAIR(Barycentric, Geocentric);
SIDERUST_CENTER_TRANSFORM_PAIR(Heliocentric, Geocentric);

#undef SIDERUST_CENTER_TRANSFORM_PAIR

template <typename From, typename To>
inline constexpr bool has_center_transform_v =
    has_center_transform<From, To>::value;

} // namespace centers
} // namespace siderust
