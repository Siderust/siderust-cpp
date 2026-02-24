#pragma once

/**
 * @file azimuth.hpp
 * @brief Azimuth computations for Sun, Moon, stars, and arbitrary ICRS directions.
 *
 * Wraps siderust-ffi's azimuth API with exception-safe C++ types and
 * RAII-managed output arrays.
 *
 * ### Covered computations
 * | Subject | azimuth_at | azimuth_crossings | azimuth_extrema | in_azimuth_range |
 * |---------|:----------:|:-----------------:|:---------------:|:----------------:|
 * | Sun     | ✓          | ✓                 | ✓               | ✓                |
 * | Moon    | ✓          | ✓                 | ✓               | ✓                |
 * | Star    | ✓          | ✓                 | –               | –                |
 * | ICRS    | ✓          | –                 | –               | –                |
 */

#include "altitude.hpp"
#include "bodies.hpp"
#include "coordinates.hpp"
#include "ffi_core.hpp"
#include "time.hpp"
#include <vector>

namespace siderust {

// ============================================================================
// Azimuth event types
// ============================================================================

/**
 * @brief Distinguishes azimuth extrema: northernmost or southernmost bearing.
 */
enum class AzimuthExtremumKind : int32_t {
    Max = 0, ///< Northernmost (or easternmost) direction reached by the body.
    Min = 1, ///< Southernmost (or westernmost) direction reached by the body.
};

/**
 * @brief An azimuth bearing-crossing event.
 */
struct AzimuthCrossingEvent {
    MJD               time;      ///< Epoch of the crossing (MJD).
    CrossingDirection direction; ///< Whether the azimuth is increasing or decreasing.

    static AzimuthCrossingEvent from_c(const siderust_azimuth_crossing_event_t& c) {
        return {MJD(c.mjd), static_cast<CrossingDirection>(c.direction)};
    }
};

/**
 * @brief An azimuth extremum event.
 */
struct AzimuthExtremum {
    MJD                time;    ///< Epoch of the extremum (MJD).
    qtty::Degree       azimuth; ///< Azimuth at the extremum (degrees, N-clockwise).
    AzimuthExtremumKind kind;   ///< Maximum or minimum.

    static AzimuthExtremum from_c(const siderust_azimuth_extremum_t& c) {
        return {MJD(c.mjd), qtty::Degree(c.azimuth_deg),
                static_cast<AzimuthExtremumKind>(c.kind)};
    }
};

// ============================================================================
// Internal helpers
// ============================================================================
namespace detail {

inline std::vector<AzimuthCrossingEvent> az_crossings_from_c(
    siderust_azimuth_crossing_event_t* ptr, uintptr_t count) {
    std::vector<AzimuthCrossingEvent> result;
    result.reserve(count);
    for (uintptr_t i = 0; i < count; ++i) {
        result.push_back(AzimuthCrossingEvent::from_c(ptr[i]));
    }
    siderust_azimuth_crossings_free(ptr, count);
    return result;
}

inline std::vector<AzimuthExtremum> az_extrema_from_c(
    siderust_azimuth_extremum_t* ptr, uintptr_t count) {
    std::vector<AzimuthExtremum> result;
    result.reserve(count);
    for (uintptr_t i = 0; i < count; ++i) {
        result.push_back(AzimuthExtremum::from_c(ptr[i]));
    }
    siderust_azimuth_extrema_free(ptr, count);
    return result;
}

} // namespace detail

// ============================================================================
// Sun azimuth
// ============================================================================

namespace sun {

/**
 * @brief Compute the Sun's azimuth (degrees, N-clockwise) at a given MJD instant.
 */
inline qtty::Degree azimuth_at(const Geodetic& obs, const MJD& mjd) {
    double out;
    check_status(siderust_sun_azimuth_at(obs.to_c(), mjd.value(), &out),
                 "sun::azimuth_at");
    return qtty::Degree(out);
}

/**
 * @brief Find epochs when the Sun crosses a given bearing.
 */
inline std::vector<AzimuthCrossingEvent> azimuth_crossings(
    const Geodetic& obs, const Period& window,
    qtty::Degree bearing, const SearchOptions& opts = {}) {
    siderust_azimuth_crossing_event_t* ptr   = nullptr;
    uintptr_t                          count = 0;
    check_status(siderust_sun_azimuth_crossings(
                     obs.to_c(), window.c_inner(), bearing.value(),
                     opts.to_c(), &ptr, &count),
                 "sun::azimuth_crossings");
    return detail::az_crossings_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<AzimuthCrossingEvent> azimuth_crossings(
    const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree bearing, const SearchOptions& opts = {}) {
    return azimuth_crossings(obs, Period(start, end), bearing, opts);
}

/**
 * @brief Find azimuth extrema (northernmost / southernmost) for the Sun.
 */
inline std::vector<AzimuthExtremum> azimuth_extrema(
    const Geodetic& obs, const Period& window,
    const SearchOptions& opts = {}) {
    siderust_azimuth_extremum_t* ptr   = nullptr;
    uintptr_t                    count = 0;
    check_status(siderust_sun_azimuth_extrema(
                     obs.to_c(), window.c_inner(),
                     opts.to_c(), &ptr, &count),
                 "sun::azimuth_extrema");
    return detail::az_extrema_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<AzimuthExtremum> azimuth_extrema(
    const Geodetic& obs, const MJD& start, const MJD& end,
    const SearchOptions& opts = {}) {
    return azimuth_extrema(obs, Period(start, end), opts);
}

/**
 * @brief Find periods when the Sun's azimuth is within [min_bearing, max_bearing].
 */
inline std::vector<Period> in_azimuth_range(
    const Geodetic& obs, const Period& window,
    qtty::Degree min_bearing, qtty::Degree max_bearing,
    const SearchOptions& opts = {}) {
    tempoch_period_mjd_t* ptr   = nullptr;
    uintptr_t             count = 0;
    check_status(siderust_sun_in_azimuth_range(
                     obs.to_c(), window.c_inner(),
                     min_bearing.value(), max_bearing.value(),
                     opts.to_c(), &ptr, &count),
                 "sun::in_azimuth_range");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<Period> in_azimuth_range(
    const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree min_bearing, qtty::Degree max_bearing,
    const SearchOptions& opts = {}) {
    return in_azimuth_range(obs, Period(start, end), min_bearing, max_bearing, opts);
}

} // namespace sun

// ============================================================================
// Moon azimuth
// ============================================================================

namespace moon {

/**
 * @brief Compute the Moon's azimuth (degrees, N-clockwise) at a given MJD instant.
 */
inline qtty::Degree azimuth_at(const Geodetic& obs, const MJD& mjd) {
    double out;
    check_status(siderust_moon_azimuth_at(obs.to_c(), mjd.value(), &out),
                 "moon::azimuth_at");
    return qtty::Degree(out);
}

/**
 * @brief Find epochs when the Moon crosses a given bearing.
 */
inline std::vector<AzimuthCrossingEvent> azimuth_crossings(
    const Geodetic& obs, const Period& window,
    qtty::Degree bearing, const SearchOptions& opts = {}) {
    siderust_azimuth_crossing_event_t* ptr   = nullptr;
    uintptr_t                          count = 0;
    check_status(siderust_moon_azimuth_crossings(
                     obs.to_c(), window.c_inner(), bearing.value(),
                     opts.to_c(), &ptr, &count),
                 "moon::azimuth_crossings");
    return detail::az_crossings_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<AzimuthCrossingEvent> azimuth_crossings(
    const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree bearing, const SearchOptions& opts = {}) {
    return azimuth_crossings(obs, Period(start, end), bearing, opts);
}

/**
 * @brief Find azimuth extrema (northernmost / southernmost) for the Moon.
 */
inline std::vector<AzimuthExtremum> azimuth_extrema(
    const Geodetic& obs, const Period& window,
    const SearchOptions& opts = {}) {
    siderust_azimuth_extremum_t* ptr   = nullptr;
    uintptr_t                    count = 0;
    check_status(siderust_moon_azimuth_extrema(
                     obs.to_c(), window.c_inner(),
                     opts.to_c(), &ptr, &count),
                 "moon::azimuth_extrema");
    return detail::az_extrema_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<AzimuthExtremum> azimuth_extrema(
    const Geodetic& obs, const MJD& start, const MJD& end,
    const SearchOptions& opts = {}) {
    return azimuth_extrema(obs, Period(start, end), opts);
}

/**
 * @brief Find periods when the Moon's azimuth is within [min_bearing, max_bearing].
 */
inline std::vector<Period> in_azimuth_range(
    const Geodetic& obs, const Period& window,
    qtty::Degree min_bearing, qtty::Degree max_bearing,
    const SearchOptions& opts = {}) {
    tempoch_period_mjd_t* ptr   = nullptr;
    uintptr_t             count = 0;
    check_status(siderust_moon_in_azimuth_range(
                     obs.to_c(), window.c_inner(),
                     min_bearing.value(), max_bearing.value(),
                     opts.to_c(), &ptr, &count),
                 "moon::in_azimuth_range");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<Period> in_azimuth_range(
    const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree min_bearing, qtty::Degree max_bearing,
    const SearchOptions& opts = {}) {
    return in_azimuth_range(obs, Period(start, end), min_bearing, max_bearing, opts);
}

} // namespace moon

// ============================================================================
// Star azimuth
// ============================================================================

namespace star_altitude {

/**
 * @brief Compute a star's azimuth (degrees, N-clockwise) at a given MJD instant.
 */
inline qtty::Degree azimuth_at(const Star& s, const Geodetic& obs, const MJD& mjd) {
    double out;
    check_status(siderust_star_azimuth_at(
                     s.c_handle(), obs.to_c(), mjd.value(), &out),
                 "star_altitude::azimuth_at");
    return qtty::Degree(out);
}

/**
 * @brief Find epochs when a star crosses a given azimuth bearing.
 */
inline std::vector<AzimuthCrossingEvent> azimuth_crossings(
    const Star& s, const Geodetic& obs, const Period& window,
    qtty::Degree bearing, const SearchOptions& opts = {}) {
    siderust_azimuth_crossing_event_t* ptr   = nullptr;
    uintptr_t                          count = 0;
    check_status(siderust_star_azimuth_crossings(
                     s.c_handle(), obs.to_c(), window.c_inner(), bearing.value(),
                     opts.to_c(), &ptr, &count),
                 "star_altitude::azimuth_crossings");
    return detail::az_crossings_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<AzimuthCrossingEvent> azimuth_crossings(
    const Star& s, const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree bearing, const SearchOptions& opts = {}) {
    return azimuth_crossings(s, obs, Period(start, end), bearing, opts);
}

} // namespace star_altitude

// ============================================================================
// ICRS direction azimuth
// ============================================================================

namespace icrs_altitude {

/**
 * @brief Compute azimuth (degrees, N-clockwise) for a fixed ICRS direction.
 */
inline qtty::Degree azimuth_at(const spherical::direction::ICRS& dir,
                               const Geodetic& obs, const MJD& mjd) {
    double out;
    check_status(siderust_icrs_azimuth_at(
                     dir.to_c(),
                     obs.to_c(), mjd.value(), &out),
                 "icrs_altitude::azimuth_at");
    return qtty::Degree(out);
}

/**
 * @brief Backward-compatible RA/Dec overload.
 */
inline qtty::Degree azimuth_at(qtty::Degree ra, qtty::Degree dec,
                               const Geodetic& obs, const MJD& mjd) {
    return azimuth_at(spherical::direction::ICRS(ra, dec), obs, mjd);
}

} // namespace icrs_altitude

} // namespace siderust
