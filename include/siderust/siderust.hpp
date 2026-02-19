#pragma once

/**
 * @file siderust.hpp
 * @brief Umbrella header for the siderust C++ wrapper library.
 *
 * Include this single header to get the full siderust C++ API.
 *
 * ## New Typed API
 *
 * @code
 * #include <siderust/siderust.hpp>
 *
 * using namespace siderust;
 * using namespace siderust::frames;
 *
 * // Typed coordinates with compile-time frame/center
 * IcrsDir vega_icrs(279.23473, 38.78369);           // Direction<ICRS>
 * auto jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
 *
 * // Template-targeted transform  — invalid pairs won't compile
 * auto ecl = vega_icrs.to_frame<EclipticMeanJ2000>(jd);   // Direction<EclipticMeanJ2000>
 * auto hor = vega_icrs.to_horizontal(jd, roque_de_los_muchachos());
 *
 * // Typed ephemeris — unit-safe AU/km positions
 * auto earth = ephemeris::earth_heliocentric_typed(jd);   // cartesian::Position<Heliocentric, EclipticMeanJ2000, AU>
 * auto dist  = earth.comp_x.to<qtty::Kilometer>();         // unit conversion
 * @endcode
 *
 * ## Legacy API (still works)
 *
 * @code
 * SphericalDirection icrs(279.23, 38.78, Frame::ICRS);
 * auto ecl = icrs.transform(Frame::EclipticMeanJ2000, jd.value());
 * @endcode
 */

#include "ffi_core.hpp"
#include "frames.hpp"
#include "centers.hpp"
#include "typed_coordinates.hpp"
#include "time.hpp"
#include "coordinates.hpp"
#include "bodies.hpp"
#include "observatories.hpp"
#include "altitude.hpp"
#include "ephemeris.hpp"
