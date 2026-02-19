#pragma once

/**
 * @file altitude.hpp
 * @brief Altitude computations for Sun, Moon, stars, and arbitrary ICRS directions.
 *
 * Wraps siderust-ffi's altitude API with exception-safe C++ types and
 * RAII-managed output arrays.
 */

#include "ffi_core.hpp"
#include "coordinates.hpp"
#include "bodies.hpp"
#include "time.hpp"
#include <vector>

namespace siderust {

// ============================================================================
// Event types
// ============================================================================

/**
 * @brief A threshold-crossing event (rising or setting).
 */
struct CrossingEvent {
    MJD               time;
    CrossingDirection  direction;

    static CrossingEvent from_c(const siderust_crossing_event_t& c) {
        return {MJD(c.mjd), static_cast<CrossingDirection>(c.direction)};
    }
};

/**
 * @brief A culmination (local altitude extremum) event.
 */
struct CulminationEvent {
    MJD              time;
    double           altitude_deg;
    CulminationKind  kind;

    static CulminationEvent from_c(const siderust_culmination_event_t& c) {
        return {MJD(c.mjd), c.altitude_deg, static_cast<CulminationKind>(c.kind)};
    }
};

// ============================================================================
// SearchOptions
// ============================================================================

/**
 * @brief Options for altitude search algorithms.
 */
struct SearchOptions {
    double time_tolerance_days = 1e-9;
    double scan_step_days      = 0.0;
    bool   has_scan_step       = false;

    SearchOptions() = default;

    /// Set a custom scan step.
    SearchOptions& with_scan_step(double step) {
        scan_step_days = step;
        has_scan_step  = true;
        return *this;
    }

    /// Set time tolerance.
    SearchOptions& with_tolerance(double tol) {
        time_tolerance_days = tol;
        return *this;
    }

    siderust_search_opts_t to_c() const {
        return {time_tolerance_days, scan_step_days, has_scan_step};
    }
};

// ============================================================================
// Internal helpers
// ============================================================================
namespace detail {

inline std::vector<Period> periods_from_c(tempoch_period_mjd_t* ptr, uintptr_t count) {
    std::vector<Period> result;
    result.reserve(count);
    for (uintptr_t i = 0; i < count; ++i) {
        result.push_back(Period::from_c(ptr[i]));
    }
    tempoch_periods_free(ptr, count);
    return result;
}

inline std::vector<CrossingEvent> crossings_from_c(siderust_crossing_event_t* ptr, uintptr_t count) {
    std::vector<CrossingEvent> result;
    result.reserve(count);
    for (uintptr_t i = 0; i < count; ++i) {
        result.push_back(CrossingEvent::from_c(ptr[i]));
    }
    siderust_crossings_free(ptr, count);
    return result;
}

inline std::vector<CulminationEvent> culminations_from_c(siderust_culmination_event_t* ptr, uintptr_t count) {
    std::vector<CulminationEvent> result;
    result.reserve(count);
    for (uintptr_t i = 0; i < count; ++i) {
        result.push_back(CulminationEvent::from_c(ptr[i]));
    }
    siderust_culminations_free(ptr, count);
    return result;
}

} // namespace detail

// ============================================================================
// Sun altitude
// ============================================================================

namespace sun {

/**
 * @brief Compute the Sun's altitude (radians) at a given MJD instant.
 */
inline double altitude_at(const Geodetic& obs, const MJD& mjd) {
    double out;
    check_status(siderust_sun_altitude_at(obs.to_c(), mjd.value(), &out),
                 "sun::altitude_at");
    return out;
}

/**
 * @brief Find periods when the Sun is above a threshold altitude.
 */
inline std::vector<Period> above_threshold(
    const Geodetic& obs, const MJD& start, const MJD& end,
    double threshold_deg, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_sun_above_threshold(
        obs.to_c(), start.value(), end.value(), threshold_deg,
        opts.to_c(), &ptr, &count),
        "sun::above_threshold");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Find periods when the Sun is below a threshold altitude.
 */
inline std::vector<Period> below_threshold(
    const Geodetic& obs, const MJD& start, const MJD& end,
    double threshold_deg, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_sun_below_threshold(
        obs.to_c(), start.value(), end.value(), threshold_deg,
        opts.to_c(), &ptr, &count),
        "sun::below_threshold");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Find threshold-crossing events for the Sun.
 */
inline std::vector<CrossingEvent> crossings(
    const Geodetic& obs, const MJD& start, const MJD& end,
    double threshold_deg, const SearchOptions& opts = {})
{
    siderust_crossing_event_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_sun_crossings(
        obs.to_c(), start.value(), end.value(), threshold_deg,
        opts.to_c(), &ptr, &count),
        "sun::crossings");
    return detail::crossings_from_c(ptr, count);
}

/**
 * @brief Find culmination events for the Sun.
 */
inline std::vector<CulminationEvent> culminations(
    const Geodetic& obs, const MJD& start, const MJD& end,
    const SearchOptions& opts = {})
{
    siderust_culmination_event_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_sun_culminations(
        obs.to_c(), start.value(), end.value(),
        opts.to_c(), &ptr, &count),
        "sun::culminations");
    return detail::culminations_from_c(ptr, count);
}

/**
 * @brief Find periods when the Sun's altitude is within [min, max].
 */
inline std::vector<Period> altitude_periods(
    const Geodetic& obs, const MJD& start, const MJD& end,
    double min_alt_deg, double max_alt_deg)
{
    siderust_altitude_query_t q = {obs.to_c(), start.value(), end.value(),
                                   min_alt_deg, max_alt_deg};
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_sun_altitude_periods(q, &ptr, &count),
                 "sun::altitude_periods");
    return detail::periods_from_c(ptr, count);
}

} // namespace sun

// ============================================================================
// Moon altitude
// ============================================================================

namespace moon {

/**
 * @brief Compute the Moon's altitude (radians) at a given MJD instant.
 */
inline double altitude_at(const Geodetic& obs, const MJD& mjd) {
    double out;
    check_status(siderust_moon_altitude_at(obs.to_c(), mjd.value(), &out),
                 "moon::altitude_at");
    return out;
}

/**
 * @brief Find periods when the Moon is above a threshold altitude.
 */
inline std::vector<Period> above_threshold(
    const Geodetic& obs, const MJD& start, const MJD& end,
    double threshold_deg, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_moon_above_threshold(
        obs.to_c(), start.value(), end.value(), threshold_deg,
        opts.to_c(), &ptr, &count),
        "moon::above_threshold");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Find periods when the Moon is below a threshold altitude.
 */
inline std::vector<Period> below_threshold(
    const Geodetic& obs, const MJD& start, const MJD& end,
    double threshold_deg, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_moon_below_threshold(
        obs.to_c(), start.value(), end.value(), threshold_deg,
        opts.to_c(), &ptr, &count),
        "moon::below_threshold");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Find threshold-crossing events for the Moon.
 */
inline std::vector<CrossingEvent> crossings(
    const Geodetic& obs, const MJD& start, const MJD& end,
    double threshold_deg, const SearchOptions& opts = {})
{
    siderust_crossing_event_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_moon_crossings(
        obs.to_c(), start.value(), end.value(), threshold_deg,
        opts.to_c(), &ptr, &count),
        "moon::crossings");
    return detail::crossings_from_c(ptr, count);
}

/**
 * @brief Find culmination events for the Moon.
 */
inline std::vector<CulminationEvent> culminations(
    const Geodetic& obs, const MJD& start, const MJD& end,
    const SearchOptions& opts = {})
{
    siderust_culmination_event_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_moon_culminations(
        obs.to_c(), start.value(), end.value(),
        opts.to_c(), &ptr, &count),
        "moon::culminations");
    return detail::culminations_from_c(ptr, count);
}

/**
 * @brief Find periods when the Moon's altitude is within [min, max].
 */
inline std::vector<Period> altitude_periods(
    const Geodetic& obs, const MJD& start, const MJD& end,
    double min_alt_deg, double max_alt_deg)
{
    siderust_altitude_query_t q = {obs.to_c(), start.value(), end.value(),
                                   min_alt_deg, max_alt_deg};
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_moon_altitude_periods(q, &ptr, &count),
                 "moon::altitude_periods");
    return detail::periods_from_c(ptr, count);
}

} // namespace moon

// ============================================================================
// Star altitude
// ============================================================================

namespace star_altitude {

/**
 * @brief Compute a star's altitude (radians) at a given MJD instant.
 */
inline double altitude_at(const Star& s, const Geodetic& obs, const MJD& mjd) {
    double out;
    check_status(siderust_star_altitude_at(
        s.c_handle(), obs.to_c(), mjd.value(), &out),
        "star_altitude::altitude_at");
    return out;
}

/**
 * @brief Find periods when a star is above a threshold altitude.
 */
inline std::vector<Period> above_threshold(
    const Star& s, const Geodetic& obs, const MJD& start, const MJD& end,
    double threshold_deg, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_star_above_threshold(
        s.c_handle(), obs.to_c(), start.value(), end.value(), threshold_deg,
        opts.to_c(), &ptr, &count),
        "star_altitude::above_threshold");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Find periods when a star is below a threshold altitude.
 */
inline std::vector<Period> below_threshold(
    const Star& s, const Geodetic& obs, const MJD& start, const MJD& end,
    double threshold_deg, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_star_below_threshold(
        s.c_handle(), obs.to_c(), start.value(), end.value(), threshold_deg,
        opts.to_c(), &ptr, &count),
        "star_altitude::below_threshold");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Find threshold-crossing events for a star.
 */
inline std::vector<CrossingEvent> crossings(
    const Star& s, const Geodetic& obs, const MJD& start, const MJD& end,
    double threshold_deg, const SearchOptions& opts = {})
{
    siderust_crossing_event_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_star_crossings(
        s.c_handle(), obs.to_c(), start.value(), end.value(), threshold_deg,
        opts.to_c(), &ptr, &count),
        "star_altitude::crossings");
    return detail::crossings_from_c(ptr, count);
}

/**
 * @brief Find culmination events for a star.
 */
inline std::vector<CulminationEvent> culminations(
    const Star& s, const Geodetic& obs, const MJD& start, const MJD& end,
    const SearchOptions& opts = {})
{
    siderust_culmination_event_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_star_culminations(
        s.c_handle(), obs.to_c(), start.value(), end.value(),
        opts.to_c(), &ptr, &count),
        "star_altitude::culminations");
    return detail::culminations_from_c(ptr, count);
}

} // namespace star_altitude

// ============================================================================
// ICRS direction altitude
// ============================================================================

namespace icrs_altitude {

/**
 * @brief Compute altitude (radians) for a fixed ICRS direction.
 */
inline double altitude_at(double ra_deg, double dec_deg,
                          const Geodetic& obs, const MJD& mjd) {
    double out;
    check_status(siderust_icrs_dir_altitude_at(
        ra_deg, dec_deg, obs.to_c(), mjd.value(), &out),
        "icrs_altitude::altitude_at");
    return out;
}

/**
 * @brief Find periods when a fixed ICRS direction is above a threshold.
 */
inline std::vector<Period> above_threshold(
    double ra_deg, double dec_deg,
    const Geodetic& obs, const MJD& start, const MJD& end,
    double threshold_deg, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_icrs_dir_above_threshold(
        ra_deg, dec_deg, obs.to_c(), start.value(), end.value(),
        threshold_deg, opts.to_c(), &ptr, &count),
        "icrs_altitude::above_threshold");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Find periods when a fixed ICRS direction is below a threshold.
 */
inline std::vector<Period> below_threshold(
    double ra_deg, double dec_deg,
    const Geodetic& obs, const MJD& start, const MJD& end,
    double threshold_deg, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_icrs_dir_below_threshold(
        ra_deg, dec_deg, obs.to_c(), start.value(), end.value(),
        threshold_deg, opts.to_c(), &ptr, &count),
        "icrs_altitude::below_threshold");
    return detail::periods_from_c(ptr, count);
}

} // namespace icrs_altitude

} // namespace siderust
