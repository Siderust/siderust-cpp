#pragma once

/**
 * @file ephemeris.hpp
 * @brief VSOP87 / ELP2000 planetary and lunar ephemeris wrappers.
 *
 * Returns compile-time typed `cartesian::Position<C, F, U>` with the
 * correct center, frame, and unit for each ephemeris query.
 */

#include "coordinates.hpp"
#include "ffi_core.hpp"
#include "time.hpp"

namespace siderust {

namespace ephemeris {

// ============================================================================
// Typed API (new) — return cartesian::Position<C, F, U>
// ============================================================================

/**
 * @brief Sun's barycentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>
sun_barycentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_sun_barycentric(jd.value(), &out),
               "ephemeris::sun_barycentric");
  return cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>::from_c(
      out);
}

/**
 * @brief Earth's barycentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::GeoBarycentric<qtty::AstronomicalUnit>
earth_barycentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_earth_barycentric(jd.value(), &out),
               "ephemeris::earth_barycentric");
  return cartesian::position::GeoBarycentric<qtty::AstronomicalUnit>::from_c(
      out);
}

/**
 * @brief Earth's heliocentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>
earth_heliocentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_earth_heliocentric(jd.value(), &out),
               "ephemeris::earth_heliocentric");
  return cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>::from_c(
      out);
}

/**
 * @brief Mars's heliocentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>
mars_heliocentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_mars_heliocentric(jd.value(), &out),
               "ephemeris::mars_heliocentric");
  return cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>::from_c(
      out);
}

/**
 * @brief Mars's barycentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>
mars_barycentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_mars_barycentric(jd.value(), &out),
               "ephemeris::mars_barycentric");
  return cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>::from_c(
      out);
}

/**
 * @brief Venus's heliocentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>
venus_heliocentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_venus_heliocentric(jd.value(), &out),
               "ephemeris::venus_heliocentric");
  return cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>::from_c(
      out);
}

/**
 * @brief Mercury's heliocentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>
mercury_heliocentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_mercury_heliocentric(jd.value(), &out),
               "ephemeris::mercury_heliocentric");
  return cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>::from_c(out);
}

/**
 * @brief Mercury's barycentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>
mercury_barycentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_mercury_barycentric(jd.value(), &out),
               "ephemeris::mercury_barycentric");
  return cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>::from_c(out);
}

/**
 * @brief Venus's barycentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>
venus_barycentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_venus_barycentric(jd.value(), &out),
               "ephemeris::venus_barycentric");
  return cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>::from_c(out);
}

/**
 * @brief Jupiter's heliocentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>
jupiter_heliocentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_jupiter_heliocentric(jd.value(), &out),
               "ephemeris::jupiter_heliocentric");
  return cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>::from_c(out);
}

/**
 * @brief Jupiter's barycentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>
jupiter_barycentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_jupiter_barycentric(jd.value(), &out),
               "ephemeris::jupiter_barycentric");
  return cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>::from_c(out);
}

/**
 * @brief Saturn's heliocentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>
saturn_heliocentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_saturn_heliocentric(jd.value(), &out),
               "ephemeris::saturn_heliocentric");
  return cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>::from_c(out);
}

/**
 * @brief Saturn's barycentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>
saturn_barycentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_saturn_barycentric(jd.value(), &out),
               "ephemeris::saturn_barycentric");
  return cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>::from_c(out);
}

/**
 * @brief Uranus's heliocentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>
uranus_heliocentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_uranus_heliocentric(jd.value(), &out),
               "ephemeris::uranus_heliocentric");
  return cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>::from_c(out);
}

/**
 * @brief Uranus's barycentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>
uranus_barycentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_uranus_barycentric(jd.value(), &out),
               "ephemeris::uranus_barycentric");
  return cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>::from_c(out);
}

/**
 * @brief Neptune's heliocentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>
neptune_heliocentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_neptune_heliocentric(jd.value(), &out),
               "ephemeris::neptune_heliocentric");
  return cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>::from_c(out);
}

/**
 * @brief Neptune's barycentric position (EclipticMeanJ2000, AU) via VSOP87.
 */
inline cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>
neptune_barycentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_neptune_barycentric(jd.value(), &out),
               "ephemeris::neptune_barycentric");
  return cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>::from_c(out);
}

/**
 * @brief Moon's geocentric position (EclipticMeanJ2000, km) via ELP2000.
 */
inline cartesian::position::MoonGeocentric<qtty::Kilometer>
moon_geocentric(const JulianDate &jd) {
  siderust_cartesian_pos_t out;
  check_status(siderust_vsop87_moon_geocentric(jd.value(), &out),
               "ephemeris::moon_geocentric");
  return cartesian::position::MoonGeocentric<qtty::Kilometer>::from_c(out);
}

} // namespace ephemeris

} // namespace siderust
