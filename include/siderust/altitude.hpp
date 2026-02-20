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
    qtty::Degree     altitude;
    CulminationKind  kind;

    static CulminationEvent from_c(const siderust_culmination_event_t& c) {
        return {MJD(c.mjd), qtty::Degree(c.altitude_deg), static_cast<CulminationKind>(c.kind)};
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
    siderust_periods_free(ptr, count);
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
inline qtty::Radian altitude_at(const Geodetic& obs, const MJD& mjd) {
    double out;
    check_status(siderust_sun_altitude_at(obs.to_c(), mjd.value(), &out),
                 "sun::altitude_at");
    return qtty::Radian(out);
}

/**
 * @brief Find periods when the Sun is above a threshold altitude.
 */
inline std::vector<Period> above_threshold(
    const Geodetic& obs, const Period& window,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_sun_above_threshold_window(
        obs.to_c(), window.c_inner(), threshold.value(),
        opts.to_c(), &ptr, &count),
        "sun::above_threshold");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<Period> above_threshold(
    const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    return above_threshold(obs, Period(start, end), threshold, opts);
}

/**
 * @brief Find periods when the Sun is below a threshold altitude.
 */
inline std::vector<Period> below_threshold(
    const Geodetic& obs, const Period& window,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_sun_below_threshold_window(
        obs.to_c(), window.c_inner(), threshold.value(),
        opts.to_c(), &ptr, &count),
        "sun::below_threshold");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<Period> below_threshold(
    const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    return below_threshold(obs, Period(start, end), threshold, opts);
}

/**
 * @brief Find threshold-crossing events for the Sun.
 */
inline std::vector<CrossingEvent> crossings(
    const Geodetic& obs, const Period& window,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    siderust_crossing_event_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_sun_crossings_window(
        obs.to_c(), window.c_inner(), threshold.value(),
        opts.to_c(), &ptr, &count),
        "sun::crossings");
    return detail::crossings_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<CrossingEvent> crossings(
    const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    return crossings(obs, Period(start, end), threshold, opts);
}

/**
 * @brief Find culmination events for the Sun.
 */
inline std::vector<CulminationEvent> culminations(
    const Geodetic& obs, const Period& window,
    const SearchOptions& opts = {})
{
    siderust_culmination_event_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_sun_culminations_window(
        obs.to_c(), window.c_inner(),
        opts.to_c(), &ptr, &count),
        "sun::culminations");
    return detail::culminations_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<CulminationEvent> culminations(
    const Geodetic& obs, const MJD& start, const MJD& end,
    const SearchOptions& opts = {})
{
    return culminations(obs, Period(start, end), opts);
}

/**
 * @brief Find periods when the Sun's altitude is within [min, max].
 */
inline std::vector<Period> altitude_periods(
    const Geodetic& obs, const Period& window,
    qtty::Degree min_alt, qtty::Degree max_alt)
{
    siderust_altitude_query_t q = {obs.to_c(), window.start().value(), window.end().value(),
                                   min_alt.value(), max_alt.value()};
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_sun_altitude_periods(q, &ptr, &count),
                 "sun::altitude_periods");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<Period> altitude_periods(
    const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree min_alt, qtty::Degree max_alt)
{
    siderust_altitude_query_t q = {obs.to_c(), start.value(), end.value(),
                                   min_alt.value(), max_alt.value()};
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
inline qtty::Radian altitude_at(const Geodetic& obs, const MJD& mjd) {
    double out;
    check_status(siderust_moon_altitude_at(obs.to_c(), mjd.value(), &out),
                 "moon::altitude_at");
    return qtty::Radian(out);
}

/**
 * @brief Find periods when the Moon is above a threshold altitude.
 */
inline std::vector<Period> above_threshold(
    const Geodetic& obs, const Period& window,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_moon_above_threshold_window(
        obs.to_c(), window.c_inner(), threshold.value(),
        opts.to_c(), &ptr, &count),
        "moon::above_threshold");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<Period> above_threshold(
    const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    return above_threshold(obs, Period(start, end), threshold, opts);
}

/**
 * @brief Find periods when the Moon is below a threshold altitude.
 */
inline std::vector<Period> below_threshold(
    const Geodetic& obs, const Period& window,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_moon_below_threshold_window(
        obs.to_c(), window.c_inner(), threshold.value(),
        opts.to_c(), &ptr, &count),
        "moon::below_threshold");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<Period> below_threshold(
    const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    return below_threshold(obs, Period(start, end), threshold, opts);
}

/**
 * @brief Find threshold-crossing events for the Moon.
 */
inline std::vector<CrossingEvent> crossings(
    const Geodetic& obs, const Period& window,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    siderust_crossing_event_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_moon_crossings_window(
        obs.to_c(), window.c_inner(), threshold.value(),
        opts.to_c(), &ptr, &count),
        "moon::crossings");
    return detail::crossings_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<CrossingEvent> crossings(
    const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    return crossings(obs, Period(start, end), threshold, opts);
}

/**
 * @brief Find culmination events for the Moon.
 */
inline std::vector<CulminationEvent> culminations(
    const Geodetic& obs, const Period& window,
    const SearchOptions& opts = {})
{
    siderust_culmination_event_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_moon_culminations_window(
        obs.to_c(), window.c_inner(),
        opts.to_c(), &ptr, &count),
        "moon::culminations");
    return detail::culminations_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<CulminationEvent> culminations(
    const Geodetic& obs, const MJD& start, const MJD& end,
    const SearchOptions& opts = {})
{
    return culminations(obs, Period(start, end), opts);
}

/**
 * @brief Find periods when the Moon's altitude is within [min, max].
 */
inline std::vector<Period> altitude_periods(
    const Geodetic& obs, const Period& window,
    qtty::Degree min_alt, qtty::Degree max_alt)
{
    siderust_altitude_query_t q = {obs.to_c(), window.start().value(), window.end().value(),
                                   min_alt.value(), max_alt.value()};
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_moon_altitude_periods(q, &ptr, &count),
                 "moon::altitude_periods");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<Period> altitude_periods(
    const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree min_alt, qtty::Degree max_alt)
{
    siderust_altitude_query_t q = {obs.to_c(), start.value(), end.value(),
                                   min_alt.value(), max_alt.value()};
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
inline qtty::Radian altitude_at(const Star& s, const Geodetic& obs, const MJD& mjd) {
    double out;
    check_status(siderust_star_altitude_at(
        s.c_handle(), obs.to_c(), mjd.value(), &out),
        "star_altitude::altitude_at");
    return qtty::Radian(out);
}

/**
 * @brief Find periods when a star is above a threshold altitude.
 */
inline std::vector<Period> above_threshold(
    const Star& s, const Geodetic& obs, const Period& window,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_star_above_threshold_window(
        s.c_handle(), obs.to_c(), window.c_inner(), threshold.value(),
        opts.to_c(), &ptr, &count),
        "star_altitude::above_threshold");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<Period> above_threshold(
    const Star& s, const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    return above_threshold(s, obs, Period(start, end), threshold, opts);
}

/**
 * @brief Find periods when a star is below a threshold altitude.
 */
inline std::vector<Period> below_threshold(
    const Star& s, const Geodetic& obs, const Period& window,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_star_below_threshold_window(
        s.c_handle(), obs.to_c(), window.c_inner(), threshold.value(),
        opts.to_c(), &ptr, &count),
        "star_altitude::below_threshold");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<Period> below_threshold(
    const Star& s, const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    return below_threshold(s, obs, Period(start, end), threshold, opts);
}

/**
 * @brief Find threshold-crossing events for a star.
 */
inline std::vector<CrossingEvent> crossings(
    const Star& s, const Geodetic& obs, const Period& window,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    siderust_crossing_event_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_star_crossings_window(
        s.c_handle(), obs.to_c(), window.c_inner(), threshold.value(),
        opts.to_c(), &ptr, &count),
        "star_altitude::crossings");
    return detail::crossings_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<CrossingEvent> crossings(
    const Star& s, const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    return crossings(s, obs, Period(start, end), threshold, opts);
}

/**
 * @brief Find culmination events for a star.
 */
inline std::vector<CulminationEvent> culminations(
    const Star& s, const Geodetic& obs, const Period& window,
    const SearchOptions& opts = {})
{
    siderust_culmination_event_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_star_culminations_window(
        s.c_handle(), obs.to_c(), window.c_inner(),
        opts.to_c(), &ptr, &count),
        "star_altitude::culminations");
    return detail::culminations_from_c(ptr, count);
}

/**
 * @brief Backward-compatible [start, end] overload.
 */
inline std::vector<CulminationEvent> culminations(
    const Star& s, const Geodetic& obs, const MJD& start, const MJD& end,
    const SearchOptions& opts = {})
{
    return culminations(s, obs, Period(start, end), opts);
}

} // namespace star_altitude

// ============================================================================
// ICRS direction altitude
// ============================================================================

namespace icrs_altitude {

/**
 * @brief Compute altitude (radians) for a fixed ICRS direction.
 */
inline qtty::Radian altitude_at(const spherical::direction::ICRS& dir,
                                const Geodetic& obs, const MJD& mjd) {
    double out;
    check_status(siderust_icrs_altitude_at(
        dir.to_c(), obs.to_c(), mjd.value(), &out),
        "icrs_altitude::altitude_at");
    return qtty::Radian(out);
}

/**
 * @brief Backward-compatible RA/Dec overload.
 */
inline qtty::Radian altitude_at(qtty::Degree ra, qtty::Degree dec,
                                const Geodetic& obs, const MJD& mjd) {
    return altitude_at(spherical::direction::ICRS(ra, dec), obs, mjd);
}

/**
 * @brief Find periods when a fixed ICRS direction is above a threshold.
 */
inline std::vector<Period> above_threshold(
    const spherical::direction::ICRS& dir,
    const Geodetic& obs, const Period& window,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_icrs_above_threshold(
        dir.to_c(), obs.to_c(), window.c_inner(),
        threshold.value(), opts.to_c(), &ptr, &count),
        "icrs_altitude::above_threshold");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Backward-compatible RA/Dec + [start, end] overload.
 */
inline std::vector<Period> above_threshold(
    qtty::Degree ra, qtty::Degree dec,
    const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    return above_threshold(
        spherical::direction::ICRS(ra, dec),
        obs,
        Period(start, end),
        threshold,
        opts
    );
}

/**
 * @brief Find periods when a fixed ICRS direction is below a threshold.
 */
inline std::vector<Period> below_threshold(
    const spherical::direction::ICRS& dir,
    const Geodetic& obs, const Period& window,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    tempoch_period_mjd_t* ptr = nullptr; uintptr_t count = 0;
    check_status(siderust_icrs_below_threshold(
        dir.to_c(), obs.to_c(), window.c_inner(),
        threshold.value(), opts.to_c(), &ptr, &count),
        "icrs_altitude::below_threshold");
    return detail::periods_from_c(ptr, count);
}

/**
 * @brief Backward-compatible RA/Dec + [start, end] overload.
 */
inline std::vector<Period> below_threshold(
    qtty::Degree ra, qtty::Degree dec,
    const Geodetic& obs, const MJD& start, const MJD& end,
    qtty::Degree threshold, const SearchOptions& opts = {})
{
    return below_threshold(
        spherical::direction::ICRS(ra, dec),
        obs,
        Period(start, end),
        threshold,
        opts
    );
}

} // namespace icrs_altitude

} // namespace siderust
