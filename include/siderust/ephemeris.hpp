#pragma once

/**
 * @file ephemeris.hpp
 * @brief VSOP87 / ELP2000 planetary and lunar ephemeris wrappers.
 *
 * Returns compile-time typed `cartesian::Position<C, F, U>` with the
 * correct center, frame, and unit for each ephemeris query.
 */

#include "ffi_core.hpp"
#include "coordinates.hpp"
#include "time.hpp"

namespace siderust {

namespace ephemeris {

// ============================================================================
// Typed API (new) — return cartesian::Position<C, F, U>
// ============================================================================

/**
 * @brief Sun's barycentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::HelioBarycentric<qtty::AstronomicalUnit> sun_barycentric(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_sun_barycentric(jd.value(), &out),
                 "ephemeris::sun_barycentric");
    return cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>::from_c(out);
}

/**
 * @brief Earth's barycentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::GeoBarycentric<qtty::AstronomicalUnit> earth_barycentric(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_earth_barycentric(jd.value(), &out),
                 "ephemeris::earth_barycentric");
    return cartesian::position::GeoBarycentric<qtty::AstronomicalUnit>::from_c(out);
}

/**
 * @brief Earth's heliocentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit> earth_heliocentric(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_earth_heliocentric(jd.value(), &out),
                 "ephemeris::earth_heliocentric");
    return cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>::from_c(out);
}

/**
 * @brief Moon's geocentric position (EclipticMeanJ2000, km) via ELP2000.
 */
inline cartesian::position::MoonGeocentric<qtty::Kilometer> moon_geocentric(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_moon_geocentric(jd.value(), &out),
                 "ephemeris::moon_geocentric");
    return cartesian::position::MoonGeocentric<qtty::Kilometer>::from_c(out);
}

} // namespace ephemeris

} // namespace siderust
