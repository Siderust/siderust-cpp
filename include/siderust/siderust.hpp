#pragma once

/**
 * @file siderust.hpp
 * @brief Umbrella header for the siderust C++ wrapper library.
 *
 * Include this single header to get the full siderust C++ API:
 *
 * @code
 * #include <siderust/siderust.hpp>
 *
 * using namespace siderust;
 *
 * auto obs   = roque_de_los_muchachos();
 * auto vega  = Star::catalog("VEGA");
 * auto jd    = JulianDate::from_utc({2026, 1, 15, 22, 0, 0});
 * auto start = MJD::from_jd(jd);
 * auto end   = start + 1.0;
 *
 * // Sun altitude at instant
 * double sun_alt = sun::altitude_at(obs, start);
 *
 * // Star above horizon
 * auto periods = star_altitude::above_threshold(vega, obs, start, end, 0.0);
 *
 * // VSOP87 Earth position
 * auto earth = ephemeris::earth_heliocentric(jd);
 * @endcode
 */

#include "ffi_core.hpp"
#include "time.hpp"
#include "coordinates.hpp"
#include "bodies.hpp"
#include "observatories.hpp"
#include "altitude.hpp"
#include "ephemeris.hpp"
