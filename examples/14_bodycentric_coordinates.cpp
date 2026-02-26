/**
 * @file 14_bodycentric_coordinates.cpp
 * @brief C++ port of siderust/examples/27_bodycentric_coordinates.rs
 *
 * Shows how to project positions into a body-centered reference frame using
 * `to_bodycentric()` and `BodycentricParams`. Useful for describing spacecraft
 * or Moon positions as seen from an orbiter.
 *
 * Run with: cmake --build build --target bodycentric_coordinates_example
 */

#include <cmath>
#include <iomanip>
#include <iostream>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;
using namespace siderust::frames;
using namespace siderust::centers;

int main() {
    std::cout << "=== Body-Centric Coordinate Transformations ===\n\n";

    const JulianDate jd = JulianDate::J2000();
    std::cout << "Epoch: J2000.0 (JD " << std::fixed << std::setprecision(1)
              << jd.value() << ")\n\n";

    // -------------------------------------------------------------------------
    // Example 1: Moon as seen from a fictitious ISS-like orbit
    // -------------------------------------------------------------------------
    std::cout << "--- Example 1: Moon from a Low-Earth Orbit ---\n";

    // ISS-like (circular low-Earth, ~400 km altitude, ~51.6° inclination)
    // a ≈ (6371 + 400) km ≈ 0.0000440 AU, e≈0, i≈51.6°
    const Orbit iss_orbit{
        0.0000440,  // semi-major axis (AU)
        0.0001,     // eccentricity
        51.6,       // inclination (°)
        0.0,        // RAAN (°)
        0.0,        // argument of periapsis (°)
        0.0,        // mean anomaly at epoch (°)
        jd.value()  // epoch (JD)
    };
    const BodycentricParams iss_params = BodycentricParams::geocentric(iss_orbit);

    // Moon's approximate geocentric position in Ecliptic J2000
    // (rough: ~0.00257 AU, ~5° ecliptic latitude)
    const cartesian::Position<Geocentric, EclipticMeanJ2000, qtty::AstronomicalUnit>
        moon_geo_pos(0.00257, 0.00034, 0.0);

    const auto moon_from_iss = to_bodycentric(moon_geo_pos, iss_params, jd);

    std::cout << "Moon from ISS (bodycentric, EclipticJ2000) [AU]:\n";
    std::cout << "  x=" << std::setprecision(6)
              << moon_from_iss.pos.x().value()
              << "  y=" << moon_from_iss.pos.y().value()
              << "  z=" << moon_from_iss.pos.z().value() << "\n";

    const double dist_au = std::sqrt(
        moon_from_iss.pos.x().value() * moon_from_iss.pos.x().value() +
        moon_from_iss.pos.y().value() * moon_from_iss.pos.y().value() +
        moon_from_iss.pos.z().value() * moon_from_iss.pos.z().value());
    std::cout << "  distance ≈ " << std::setprecision(6) << dist_au << " AU  ("
              << std::setprecision(0) << dist_au * 1.496e8 << " km)\n\n";

    // Round-trip back to geocentric
    const auto recovered = moon_from_iss.to_geocentric(jd);
    std::cout << "Round-trip to geocentric [AU]:\n";
    std::cout << "  x=" << std::setprecision(6) << recovered.x().value()
              << "  y=" << recovered.y().value()
              << "  z=" << recovered.z().value() << "\n";

    const double err = std::sqrt(
        std::pow(recovered.x().value() - moon_geo_pos.x().value(), 2) +
        std::pow(recovered.y().value() - moon_geo_pos.y().value(), 2) +
        std::pow(recovered.z().value() - moon_geo_pos.z().value(), 2));
    std::cout << "  round-trip error = " << std::setprecision(2) << std::scientific
              << err << " AU\n\n";

    // -------------------------------------------------------------------------
    // Example 2: Mars Phobos-like orbit
    // -------------------------------------------------------------------------
    std::cout << "--- Example 2: Position Relative to Mars (Phobos-like Orbit) ---\n";

    // Phobos: a ≈ 9376 km ≈ 0.0000627 AU, e≈0.015, i≈1.1°
    const Orbit phobos_orbit{
        0.0000627,   // semi-major axis (AU)
        0.015,       // eccentricity
        1.1,         // inclination (°)
        0.0,         // RAAN (°)
        0.0,         // argument of periapsis (°)
        5.0,         // mean anomaly at epoch (°)
        jd.value()   // epoch (JD)
    };
    const BodycentricParams phobos_params = BodycentricParams::heliocentric(phobos_orbit);

    // Mars heliocentric position at J2000.0 (approximate)
    const auto mars_hel = ephemeris::mars_heliocentric(jd);

    const auto phobos_from_mars = to_bodycentric(mars_hel, phobos_params, jd);

    std::cout << "Mars heliocentric position [AU]:"
              << "  r=" << std::setprecision(3)
              << mars_hel.to_spherical().distance().value() << "\n";
    std::cout << "Phobos bodycentric (relative to Mars) [AU]:\n";
    std::cout << "  x=" << std::setprecision(8) << phobos_from_mars.pos.x().value()
              << "  y=" << phobos_from_mars.pos.y().value()
              << "  z=" << phobos_from_mars.pos.z().value() << "\n";

    // -------------------------------------------------------------------------
    // Example 3: Geocentric orbit (circular, equatorial)
    // -------------------------------------------------------------------------
    std::cout << "\n--- Example 3: GEO Satellite ---\n";

    const Orbit geo_orbit{
        0.000284,   // ~42164 km ≈ 0.000282 AU
        0.0001,     // nearly circular
        0.1,        // near-equatorial
        120.0,      // RAAN
        0.0,        // arg periapsis
        0.0,        // mean anomaly
        jd.value()
    };
    const BodycentricParams geo_params = BodycentricParams::geocentric(geo_orbit);

    // Sun geocentric approximate position
    const auto sun_geo_approx =
        cartesian::Position<Geocentric, EclipticMeanJ2000, qtty::AstronomicalUnit>(
            -1.0, 0.0, 0.0); // rough

    const auto sun_from_geo = to_bodycentric(sun_geo_approx, geo_params, jd);
    const double sun_dist_au = std::sqrt(
        sun_from_geo.pos.x().value() * sun_from_geo.pos.x().value() +
        sun_from_geo.pos.y().value() * sun_from_geo.pos.y().value() +
        sun_from_geo.pos.z().value() * sun_from_geo.pos.z().value());

    std::cout << "Sun as seen from GEO satellite [AU]: r=" << std::setprecision(4)
              << sun_dist_au << "\n";

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
