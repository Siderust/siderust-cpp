#pragma once

/**
 * @file ephemeris.hpp
 * @brief VSOP87 / ELP2000 planetary and lunar ephemeris wrappers.
 *
 * Returns compile-time typed `cartesian::Position<C, F, U>` with the
 * correct center, frame, and unit for each ephemeris query.
 *
 * Also provides legacy overloads returning `CartesianPosition` for
 * backward compatibility.
 */

#include "ffi_core.hpp"
#include "typed_coordinates.hpp"
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
inline HelioBaryCartPosAU sun_barycentric_typed(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_sun_barycentric(jd.value(), &out),
                 "ephemeris::sun_barycentric");
    return HelioBaryCartPosAU::from_c(out);
}

/**
 * @brief Earth's barycentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline GeoBaryCartPosAU earth_barycentric_typed(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_earth_barycentric(jd.value(), &out),
                 "ephemeris::earth_barycentric");
    return GeoBaryCartPosAU::from_c(out);
}

/**
 * @brief Earth's heliocentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline EclipticCartPosAU earth_heliocentric_typed(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_earth_heliocentric(jd.value(), &out),
                 "ephemeris::earth_heliocentric");
    return EclipticCartPosAU::from_c(out);
}

/**
 * @brief Moon's geocentric position (EclipticMeanJ2000, km) via ELP2000.
 */
inline MoonGeoCartPosKM moon_geocentric_typed(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_moon_geocentric(jd.value(), &out),
                 "ephemeris::moon_geocentric");
    return MoonGeoCartPosKM::from_c(out);
}

// ============================================================================
// Legacy API (backward compatible) — return CartesianPosition
// ============================================================================

/**
 * @brief Sun's barycentric position (legacy, runtime-tagged).
 */
inline CartesianPosition sun_barycentric(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_sun_barycentric(jd.value(), &out),
                 "ephemeris::sun_barycentric");
    return CartesianPosition::from_c(out);
}

/**
 * @brief Earth's barycentric position (legacy, runtime-tagged).
 */
inline CartesianPosition earth_barycentric(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_earth_barycentric(jd.value(), &out),
                 "ephemeris::earth_barycentric");
    return CartesianPosition::from_c(out);
}

/**
 * @brief Earth's heliocentric position (legacy, runtime-tagged).
 */
inline CartesianPosition earth_heliocentric(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_earth_heliocentric(jd.value(), &out),
                 "ephemeris::earth_heliocentric");
    return CartesianPosition::from_c(out);
}

/**
 * @brief Moon's geocentric position (legacy, runtime-tagged).
 */
inline CartesianPosition moon_geocentric(const JulianDate& jd) {
    siderust_cartesian_pos_t out;
    check_status(siderust_vsop87_moon_geocentric(jd.value(), &out),
                 "ephemeris::moon_geocentric");
    return CartesianPosition::from_c(out);
}

} // namespace ephemeris

} // namespace siderust
