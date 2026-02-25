#pragma once

/**
 * @file siderust.hpp
 * @brief Umbrella header for the siderust C++ wrapper library.
 *
 * Include this single header to get the full siderust C++ API.
 *
 * @code
 * #include <siderust/siderust.hpp>
 *
 * using namespace siderust;
 * using namespace siderust::frames;
 *
 * // Typed coordinates with compile-time frame/center
 * spherical::direction::ICRS vega_icrs(qtty::Degree(279.23473),
 * qtty::Degree(38.78369)); auto jd = JulianDate::from_utc({2026, 7, 15, 22, 0,
 * 0});
 *
 * // Template-targeted transform  — invalid pairs won't compile
 * auto ecl = vega_icrs.to_frame<EclipticMeanJ2000>(jd);   //
 * Direction<EclipticMeanJ2000> auto hor = vega_icrs.to_horizontal(jd,
 * ROQUE_DE_LOS_MUCHACHOS);
 *
 * // Typed ephemeris — unit-safe AU/km positions
 * auto earth = ephemeris::earth_heliocentric(jd);   //
 * cartesian::Position<Heliocentric, EclipticMeanJ2000, AU> auto dist  =
 * earth.comp_x.to<qtty::Kilometer>();   // unit conversion
 * @endcode
 */

#include "altitude.hpp"
#include "azimuth.hpp"
#include "bodies.hpp"
#include "body_target.hpp"
#include "centers.hpp"
#include "coordinates.hpp"
#include "ephemeris.hpp"
#include "ffi_core.hpp"
#include "frames.hpp"
#include "lunar_phase.hpp"
#include "observatories.hpp"
#include "star_target.hpp"
#include "target.hpp"
#include "time.hpp"
