#pragma once

/**
 * @file frames.hpp
 * @brief Zero-sized reference-frame tag types mirroring Rust's affn::frames.
 *
 * Each frame is a unique empty struct used as a compile-time tag for
 * coordinate templates.  Every tag carries a static `ffi_id` that maps
 * to the runtime `siderust_frame_t` C enum, and a human-readable name.
 */

#include "ffi_core.hpp"

namespace siderust {
namespace frames {

// ============================================================================
// Frame Trait
// ============================================================================

/**
 * @brief SFINAE helper: every frame tag must provide these static members.
 *
 * - `ffi_id`   — the C FFI enum value
 * - `name()`   — human-readable frame name
 */
template<typename F>
struct FrameTraits;  // primary template — intentionally undefined

/**
 * @brief Concept-like compile-time check (C++17: constexpr bool).
 */
template<typename F, typename = void>
struct is_frame : std::false_type {};

template<typename F>
struct is_frame<F, std::void_t<decltype(FrameTraits<F>::ffi_id)>> : std::true_type {};

template<typename F>
inline constexpr bool is_frame_v = is_frame<F>::value;

// ============================================================================
// Frame Tag Definitions
// ============================================================================

/// International Celestial Reference System.
struct ICRS {};
/// International Celestial Reference Frame (treated ≡ ICRS).
struct ICRF {};
/// Mean ecliptic & equinox of J2000.0.
struct EclipticMeanJ2000 {};
/// Ecliptic of date (precessed mean obliquity, no nutation).
struct EclipticOfDate {};
/// True ecliptic of date (precessed + nutated).
struct EclipticTrueOfDate {};
/// Mean equatorial of J2000.0 (FK5-aligned).
struct EquatorialMeanJ2000 {};
/// Mean equatorial of date (precessed, no nutation).
struct EquatorialMeanOfDate {};
/// True equatorial of date (precessed + nutated).
struct EquatorialTrueOfDate {};
/// Local horizontal (topocentric alt-az).
struct Horizontal {};
/// Galactic coordinate system (IAU 1958).
struct Galactic {};
/// Earth-Centered Earth-Fixed.
struct ECEF {};
/// International Terrestrial Reference Frame.
struct ITRF {};
/// Celestial Intermediate Reference System.
struct CIRS {};
/// Geocentric Celestial Reference System.
struct GCRS {};
/// Terrestrial Intermediate Reference System.
struct TIRS {};
/// Ecliptic mean of date (alias for EclipticOfDate).
struct EclipticMeanOfDate {};

// ============================================================================
// FrameTraits Specializations
// ============================================================================

#define SIDERUST_DEFINE_FRAME(Tag, EnumVal, Label)         \
    template<> struct FrameTraits<Tag> {                   \
        static constexpr siderust_frame_t ffi_id = EnumVal;\
        static constexpr const char* name() { return Label; } \
    }

SIDERUST_DEFINE_FRAME(ICRS,                   SIDERUST_FRAME_T_ICRS,                    "ICRS");
SIDERUST_DEFINE_FRAME(ICRF,                   SIDERUST_FRAME_T_ICRF,                    "ICRF");
SIDERUST_DEFINE_FRAME(EclipticMeanJ2000,      SIDERUST_FRAME_T_ECLIPTIC_MEAN_J2000,     "EclipticMeanJ2000");
SIDERUST_DEFINE_FRAME(EclipticOfDate,         SIDERUST_FRAME_T_ECLIPTIC_OF_DATE,         "EclipticOfDate");
SIDERUST_DEFINE_FRAME(EclipticTrueOfDate,     SIDERUST_FRAME_T_ECLIPTIC_TRUE_OF_DATE,    "EclipticTrueOfDate");
SIDERUST_DEFINE_FRAME(EquatorialMeanJ2000,    SIDERUST_FRAME_T_EQUATORIAL_MEAN_J2000,    "EquatorialMeanJ2000");
SIDERUST_DEFINE_FRAME(EquatorialMeanOfDate,   SIDERUST_FRAME_T_EQUATORIAL_MEAN_OF_DATE,  "EquatorialMeanOfDate");
SIDERUST_DEFINE_FRAME(EquatorialTrueOfDate,   SIDERUST_FRAME_T_EQUATORIAL_TRUE_OF_DATE,  "EquatorialTrueOfDate");
SIDERUST_DEFINE_FRAME(Horizontal,             SIDERUST_FRAME_T_HORIZONTAL,               "Horizontal");
SIDERUST_DEFINE_FRAME(Galactic,               SIDERUST_FRAME_T_GALACTIC,                 "Galactic");
SIDERUST_DEFINE_FRAME(ECEF,                   SIDERUST_FRAME_T_ECEF,                     "ECEF");
SIDERUST_DEFINE_FRAME(ITRF,                   SIDERUST_FRAME_T_ITRF,                     "ITRF");
SIDERUST_DEFINE_FRAME(CIRS,                   SIDERUST_FRAME_T_CIRS,                     "CIRS");
SIDERUST_DEFINE_FRAME(GCRS,                   SIDERUST_FRAME_T_GCRS,                     "GCRS");
SIDERUST_DEFINE_FRAME(TIRS,                   SIDERUST_FRAME_T_TIRS,                     "TIRS");

#undef SIDERUST_DEFINE_FRAME

// ============================================================================
// Naming Conventions (Spherical)
// ============================================================================

/**
 * @brief Maps a frame to its conventional spherical-coordinate names.
 *
 * Default: (longitude, latitude).  Specialise per-frame for RA/Dec, Az/Alt, etc.
 */
template<typename F>
struct SphericalNaming {
    static constexpr const char* lon_name() { return "longitude"; }
    static constexpr const char* lat_name() { return "latitude"; }
};

template<>
struct SphericalNaming<ICRS> {
    static constexpr const char* lon_name() { return "right_ascension"; }
    static constexpr const char* lat_name() { return "declination"; }
};

template<>
struct SphericalNaming<ICRF> {
    static constexpr const char* lon_name() { return "right_ascension"; }
    static constexpr const char* lat_name() { return "declination"; }
};

template<>
struct SphericalNaming<EquatorialMeanJ2000> {
    static constexpr const char* lon_name() { return "right_ascension"; }
    static constexpr const char* lat_name() { return "declination"; }
};

template<>
struct SphericalNaming<EquatorialMeanOfDate> {
    static constexpr const char* lon_name() { return "right_ascension"; }
    static constexpr const char* lat_name() { return "declination"; }
};

template<>
struct SphericalNaming<EquatorialTrueOfDate> {
    static constexpr const char* lon_name() { return "right_ascension"; }
    static constexpr const char* lat_name() { return "declination"; }
};

template<>
struct SphericalNaming<Horizontal> {
    static constexpr const char* lon_name() { return "azimuth"; }
    static constexpr const char* lat_name() { return "altitude"; }
};

template<>
struct SphericalNaming<Galactic> {
    static constexpr const char* lon_name() { return "l"; }
    static constexpr const char* lat_name() { return "b"; }
};

template<>
struct SphericalNaming<EclipticMeanJ2000> {
    static constexpr const char* lon_name() { return "ecliptic_longitude"; }
    static constexpr const char* lat_name() { return "ecliptic_latitude"; }
};

// ============================================================================
// Transform-Valid Predicate
// ============================================================================

/**
 * @brief Marks frame pairs for which a FrameRotationProvider exists in
 *        siderust-ffi.
 *
 * The FFI layer supports a hub-and-spoke model with hub = ICRS.
 * Supported source/target frames: ICRS, EclipticMeanJ2000,
 * EquatorialMeanJ2000, EquatorialMeanOfDate, EquatorialTrueOfDate.
 *
 * Same-frame identity transforms are always valid.
 */
template<typename From, typename To>
struct has_frame_transform : std::false_type {};

// Identity
template<typename F>
struct has_frame_transform<F, F> : std::true_type {};

// Hub spokes (bidirectional)
#define SIDERUST_FRAME_TRANSFORM_PAIR(A, B)           \
    template<> struct has_frame_transform<A, B> : std::true_type {}; \
    template<> struct has_frame_transform<B, A> : std::true_type {}

// All pairs reachable through the ICRS hub
SIDERUST_FRAME_TRANSFORM_PAIR(ICRS, EclipticMeanJ2000);
SIDERUST_FRAME_TRANSFORM_PAIR(ICRS, EquatorialMeanJ2000);
SIDERUST_FRAME_TRANSFORM_PAIR(ICRS, EquatorialMeanOfDate);
SIDERUST_FRAME_TRANSFORM_PAIR(ICRS, EquatorialTrueOfDate);
SIDERUST_FRAME_TRANSFORM_PAIR(EclipticMeanJ2000, EquatorialMeanJ2000);
SIDERUST_FRAME_TRANSFORM_PAIR(EclipticMeanJ2000, EquatorialMeanOfDate);
SIDERUST_FRAME_TRANSFORM_PAIR(EclipticMeanJ2000, EquatorialTrueOfDate);
SIDERUST_FRAME_TRANSFORM_PAIR(EquatorialMeanJ2000, EquatorialMeanOfDate);
SIDERUST_FRAME_TRANSFORM_PAIR(EquatorialMeanJ2000, EquatorialTrueOfDate);
SIDERUST_FRAME_TRANSFORM_PAIR(EquatorialMeanOfDate, EquatorialTrueOfDate);
// ICRF ≡ ICRS
SIDERUST_FRAME_TRANSFORM_PAIR(ICRF, EclipticMeanJ2000);
SIDERUST_FRAME_TRANSFORM_PAIR(ICRF, EquatorialMeanJ2000);
SIDERUST_FRAME_TRANSFORM_PAIR(ICRF, EquatorialMeanOfDate);
SIDERUST_FRAME_TRANSFORM_PAIR(ICRF, EquatorialTrueOfDate);
SIDERUST_FRAME_TRANSFORM_PAIR(ICRF, ICRS);

#undef SIDERUST_FRAME_TRANSFORM_PAIR

template<typename From, typename To>
inline constexpr bool has_frame_transform_v = has_frame_transform<From, To>::value;

/**
 * @brief Marks frames from which to_horizontal is reachable.
 */
template<typename F>
struct has_horizontal_transform : std::false_type {};

template<> struct has_horizontal_transform<ICRS> : std::true_type {};
template<> struct has_horizontal_transform<ICRF> : std::true_type {};
template<> struct has_horizontal_transform<EclipticMeanJ2000> : std::true_type {};
template<> struct has_horizontal_transform<EquatorialMeanJ2000> : std::true_type {};
template<> struct has_horizontal_transform<EquatorialMeanOfDate> : std::true_type {};
template<> struct has_horizontal_transform<EquatorialTrueOfDate> : std::true_type {};

template<typename F>
inline constexpr bool has_horizontal_transform_v = has_horizontal_transform<F>::value;

} // namespace frames
} // namespace siderust
