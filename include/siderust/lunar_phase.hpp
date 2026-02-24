#pragma once

/**
 * @file lunar_phase.hpp
 * @brief Lunar phase geometry, phase events, and illumination periods.
 *
 * Wraps siderust-ffi's lunar phase API with exception-safe C++ types and
 * RAII-managed output arrays.
 *
 * All phase-geometry functions accept a Julian Date (siderust::JulianDate).
 * Search windows use the regular MJD-based siderust::Period.
 */

#include "altitude.hpp"
#include "coordinates.hpp"
#include "ffi_core.hpp"
#include "time.hpp"
#include <vector>

namespace siderust {

// ============================================================================
// Phase enumerations
// ============================================================================

/**
 * @brief Principal lunar phase kinds (new-moon quarter events).
 */
enum class PhaseKind : int32_t {
    NewMoon      = 0,
    FirstQuarter = 1,
    FullMoon     = 2,
    LastQuarter  = 3,
};

/**
 * @brief Descriptive moon phase labels (8 canonical phases).
 */
enum class MoonPhaseLabel : int32_t {
    NewMoon         = 0,
    WaxingCrescent  = 1,
    FirstQuarter    = 2,
    WaxingGibbous   = 3,
    FullMoon        = 4,
    WaningGibbous   = 5,
    LastQuarter     = 6,
    WaningCrescent  = 7,
};

// ============================================================================
// Phase event / geometry types
// ============================================================================

/**
 * @brief Geometric description of the Moon's phase at a point in time.
 */
struct MoonPhaseGeometry {
    double phase_angle_rad;      ///< Phase angle in [0, π], radians.
    double illuminated_fraction; ///< Illuminated disc fraction in [0, 1].
    double elongation_rad;       ///< Sun–Moon elongation, radians.
    bool   waxing;               ///< True when the Moon is waxing.

    static MoonPhaseGeometry from_c(const siderust_moon_phase_geometry_t& c) {
        return {c.phase_angle_rad, c.illuminated_fraction,
                c.elongation_rad, static_cast<bool>(c.waxing)};
    }
};

/**
 * @brief A principal lunar phase event (new moon, first quarter, etc.).
 */
struct PhaseEvent {
    MJD       time; ///< Epoch of the event (MJD).
    PhaseKind kind; ///< Which principal phase occurred.

    static PhaseEvent from_c(const siderust_phase_event_t& c) {
        return {MJD(c.mjd), static_cast<PhaseKind>(c.kind)};
    }
};

// ============================================================================
// Internal helpers
// ============================================================================
namespace detail {

inline std::vector<PhaseEvent> phase_events_from_c(
    siderust_phase_event_t* ptr, uintptr_t count) {
    std::vector<PhaseEvent> result;
    result.reserve(count);
    for (uintptr_t i = 0; i < count; ++i) {
        result.push_back(PhaseEvent::from_c(ptr[i]));
    }
    siderust_phase_events_free(ptr, count);
    return result;
}

/// Like periods_from_c but for tempoch_period_mjd_t* pointers (freed with
/// siderust_periods_free).
inline std::vector<Period> illum_periods_from_c(
    tempoch_period_mjd_t* ptr, uintptr_t count) {
    std::vector<Period> result;
    result.reserve(count);
    for (uintptr_t i = 0; i < count; ++i) {
        result.push_back(Period(MJD(ptr[i].start_mjd), MJD(ptr[i].end_mjd)));
    }
    siderust_periods_free(ptr, count);
    return result;
}

} // namespace detail

// ============================================================================
// Lunar phase namespace
// ============================================================================

namespace moon {

/**
 * @brief Compute geocentric Moon phase geometry at a Julian Date.
 *
 * @param jd  Julian Date (e.g. `siderust::JulianDate(2451545.0)` for J2000.0).
 */
inline MoonPhaseGeometry phase_geocentric(const JulianDate& jd) {
    siderust_moon_phase_geometry_t out{};
    check_status(siderust_moon_phase_geocentric(jd.value(), &out),
                 "moon::phase_geocentric");
    return MoonPhaseGeometry::from_c(out);
}

/**
 * @brief Compute topocentric Moon phase geometry at a Julian Date.
 *
 * @param jd    Julian Date.
 * @param site  Observer geodetic coordinates.
 */
inline MoonPhaseGeometry phase_topocentric(const JulianDate& jd,
                                           const Geodetic&   site) {
    siderust_moon_phase_geometry_t out{};
    check_status(siderust_moon_phase_topocentric(jd.value(), site.to_c(), &out),
                 "moon::phase_topocentric");
    return MoonPhaseGeometry::from_c(out);
}

/**
 * @brief Determine the descriptive phase label for a given geometry.
 *
 * @param geom  Moon phase geometry (as returned by phase_geocentric / phase_topocentric).
 */
inline MoonPhaseLabel phase_label(const MoonPhaseGeometry& geom) {
    siderust_moon_phase_geometry_t c{geom.phase_angle_rad,
                                     geom.illuminated_fraction,
                                     geom.elongation_rad,
                                     static_cast<uint8_t>(geom.waxing)};
    siderust_moon_phase_label_t out{};
    check_status(siderust_moon_phase_label(c, &out), "moon::phase_label");
    return static_cast<MoonPhaseLabel>(out);
}

/**
 * @brief Find principal phase events (new moon, quarters, full moon) in a window.
 *
 * @param window  MJD search window.
 * @param opts    Search tolerances (optional).
 */
inline std::vector<PhaseEvent> find_phase_events(
    const Period& window, const SearchOptions& opts = {}) {
    siderust_phase_event_t* ptr   = nullptr;
    uintptr_t               count = 0;
    check_status(siderust_find_phase_events(
                     window.c_inner(), opts.to_c(), &ptr, &count),
                 "moon::find_phase_events");
    return detail::phase_events_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<PhaseEvent> find_phase_events(
    const MJD& start, const MJD& end, const SearchOptions& opts = {}) {
    return find_phase_events(Period(start, end), opts);
}

/**
 * @brief Find periods when illuminated fraction is ≥ k_min.
 *
 * @param window  MJD search window.
 * @param k_min   Minimum illuminated fraction in [0, 1].
 * @param opts    Search tolerances (optional).
 */
inline std::vector<Period> illumination_above(
    const Period& window, double k_min, const SearchOptions& opts = {}) {
    tempoch_period_mjd_t* ptr   = nullptr;
    uintptr_t              count = 0;
    check_status(siderust_moon_illumination_above(
                     window.c_inner(), k_min, opts.to_c(), &ptr, &count),
                 "moon::illumination_above");
    return detail::illum_periods_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<Period> illumination_above(
    const MJD& start, const MJD& end, double k_min,
    const SearchOptions& opts = {}) {
    return illumination_above(Period(start, end), k_min, opts);
}

/**
 * @brief Find periods when illuminated fraction is ≤ k_max.
 *
 * @param window  MJD search window.
 * @param k_max   Maximum illuminated fraction in [0, 1].
 * @param opts    Search tolerances (optional).
 */
inline std::vector<Period> illumination_below(
    const Period& window, double k_max, const SearchOptions& opts = {}) {
    tempoch_period_mjd_t* ptr   = nullptr;
    uintptr_t              count = 0;
    check_status(siderust_moon_illumination_below(
                     window.c_inner(), k_max, opts.to_c(), &ptr, &count),
                 "moon::illumination_below");
    return detail::illum_periods_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<Period> illumination_below(
    const MJD& start, const MJD& end, double k_max,
    const SearchOptions& opts = {}) {
    return illumination_below(Period(start, end), k_max, opts);
}

/**
 * @brief Find periods when illuminated fraction is within [k_min, k_max].
 *
 * @param window  MJD search window.
 * @param k_min   Minimum illuminated fraction in [0, 1].
 * @param k_max   Maximum illuminated fraction in [0, 1].
 * @param opts    Search tolerances (optional).
 */
inline std::vector<Period> illumination_range(
    const Period& window, double k_min, double k_max,
    const SearchOptions& opts = {}) {
    tempoch_period_mjd_t* ptr   = nullptr;
    uintptr_t              count = 0;
    check_status(siderust_moon_illumination_range(
                     window.c_inner(), k_min, k_max, opts.to_c(), &ptr, &count),
                 "moon::illumination_range");
    return detail::illum_periods_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<Period> illumination_range(
    const MJD& start, const MJD& end, double k_min, double k_max,
    const SearchOptions& opts = {}) {
    return illumination_range(Period(start, end), k_min, k_max, opts);
}

} // namespace moon

// ============================================================================
// Convenience helpers (pure C++, no FFI)
// ============================================================================

/**
 * @brief Get the illuminated fraction as a percentage [0, 100].
 */
inline double illuminated_percent(const MoonPhaseGeometry& geom) {
    return geom.illuminated_fraction * 100.0;
}

/**
 * @brief Check if a phase label describes a waxing moon.
 */
inline bool is_waxing(MoonPhaseLabel label) {
    switch (label) {
    case MoonPhaseLabel::WaxingCrescent:
    case MoonPhaseLabel::FirstQuarter:
    case MoonPhaseLabel::WaxingGibbous:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Check if a phase label describes a waning moon.
 */
inline bool is_waning(MoonPhaseLabel label) {
    switch (label) {
    case MoonPhaseLabel::WaningGibbous:
    case MoonPhaseLabel::LastQuarter:
    case MoonPhaseLabel::WaningCrescent:
        return true;
    default:
        return false;
    }
}

} // namespace siderust
