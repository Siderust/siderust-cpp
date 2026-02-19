#pragma once

/**
 * @file ephemeris.hpp
 * @brief VSOP87 / ELP2000 planetary and lunar ephemeris wrappers.
 */

#include "ffi_core.hpp"
#include "coordinates.hpp"
#include "time.hpp"

namespace siderust {

namespace ephemeris {

/**
 * @brief Sun's barycentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline CartesianPosition sun_barycentric(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_sun_barycentric(jd.value(), &out),
                 "ephemeris::sun_barycentric");
    return CartesianPosition::from_c(out);
}

/**
 * @brief Earth's barycentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline CartesianPosition earth_barycentric(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_earth_barycentric(jd.value(), &out),
                 "ephemeris::earth_barycentric");
    return CartesianPosition::from_c(out);
}

/**
 * @brief Earth's heliocentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline CartesianPosition earth_heliocentric(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_earth_heliocentric(jd.value(), &out),
                 "ephemeris::earth_heliocentric");
    return CartesianPosition::from_c(out);
}

/**
 * @brief Moon's geocentric position (EclipticMeanJ2000, km) via ELP2000.
 */
inline CartesianPosition moon_geocentric(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_moon_geocentric(jd.value(), &out),
                 "ephemeris::moon_geocentric");
    return CartesianPosition::from_c(out);
}

} // namespace ephemeris

} // namespace siderust
