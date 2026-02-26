/**
 * @file 12_solar_system_example.cpp
 * @brief C++ port of siderust/examples/38_solar_system_example.rs
 *
 * Computes heliocentric and barycentric positions for solar-system bodies
 * using the VSOP87 ephemeris layer exposed by siderust.
 *
 * Bodies with VSOP87 bindings in C++: Sun, Earth, Mars, Venus, Moon.
 * (Mercury/Jupiter/Saturn/Uranus/Neptune not yet bound in C++ FFI.)
 *
 * Run with: cmake --build build --target 12_solar_system_example
 */

#include <cmath>
#include <iomanip>
#include <iostream>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

int main() {
    std::cout << "=== Solar System Bodies Positions ===\n\n";

    const JulianDate jd(2451545.0);
    std::cout << "Epoch: JD " << std::fixed << std::setprecision(1)
              << jd.value() << "  (J2000.0)\n\n";

    // -------------------------------------------------------------------------
    // Heliocentric positions (EclipticMeanJ2000 frame, AU)
    // -------------------------------------------------------------------------
    std::cout << "--- Heliocentric Positions (VSOP87) ---\n";

    auto print_hel = [](const char *name, auto pos) {
        const auto sph = pos.to_spherical();
        const auto dir = sph.direction();
        std::cout << std::left << std::setw(8) << name
                  << "  lon=" << std::right << std::fixed << std::setprecision(3)
                  << std::setw(10) << dir.longitude().value() << " deg"
                  << "  lat=" << std::setw(8) << dir.latitude().value() << " deg"
                  << "  r=" << std::setw(12) << std::setprecision(6)
                  << sph.distance().value() << " AU\n";
    };

    const auto earth_h = ephemeris::earth_heliocentric(jd);
    const auto venus_h = ephemeris::venus_heliocentric(jd);
    const auto mars_h  = ephemeris::mars_heliocentric(jd);

    print_hel("Earth",  earth_h);
    print_hel("Venus",  venus_h);
    print_hel("Mars",   mars_h);

    std::cout << "\n  [NOTE: Mercury, Jupiter, Saturn, Uranus, Neptune heliocentric\n";
    std::cout << "   not yet bound in C++ FFI.]\n\n";

    // -------------------------------------------------------------------------
    // Barycentric positions (Solar System Barycenter origin, AU)
    // -------------------------------------------------------------------------
    std::cout << "--- Barycentric Positions ---\n";

    const auto sun_bc   = ephemeris::sun_barycentric(jd);
    const auto earth_bc = ephemeris::earth_barycentric(jd);

    auto print_bary = [](const char *name, auto pos) {
        const auto sph = pos.to_spherical();
        std::cout << std::left << std::setw(16) << name
                  << "  r=" << std::right << std::fixed << std::setprecision(6)
                  << sph.distance().value() << " AU\n";
    };

    print_bary("Sun (SSB)",    sun_bc);
    print_bary("Earth (SSB)",  earth_bc);

    std::cout << "\n  [NOTE: mars_barycentric, moon_barycentric not yet bound.]\n\n";

    // -------------------------------------------------------------------------
    // Moon geocentric (km)
    // -------------------------------------------------------------------------
    std::cout << "--- Moon Geocentric Position ---\n";
    const auto moon_g = ephemeris::moon_geocentric(jd);
    {
        const auto sph = moon_g.to_spherical();
        const auto dir = sph.direction();
        const double r_km = sph.distance().value();
        std::cout << "Moon  r=" << std::setprecision(1) << r_km << " km"
                  << "  (" << std::setprecision(6) << r_km / 1.496e8 << " AU)"
                  << "  lon=" << std::setprecision(3) << dir.longitude().value() << " deg"
                  << "  lat=" << dir.latitude().value() << " deg\n\n";
    }

    // -------------------------------------------------------------------------
    // Planet catalog (orbital elements)
    // -------------------------------------------------------------------------
    std::cout << "--- Planet Catalog (orbital elements) ---\n";

    struct PlanetInfo { const char *name; const Planet *planet; };
    const PlanetInfo catalog[] = {
        {"Mercury", &MERCURY},
        {"Venus",   &VENUS},
        {"Earth",   &EARTH},
        {"Mars",    &MARS},
        {"Jupiter", &JUPITER},
        {"Saturn",  &SATURN},
        {"Uranus",  &URANUS},
        {"Neptune", &NEPTUNE},
    };

    std::cout << std::setw(10) << "Planet"
              << std::setw(12) << "a (AU)"
              << std::setw(10) << "e"
              << std::setw(12) << "r_body (km)\n";
    std::cout << std::string(44, '-') << "\n";

    for (const auto &p : catalog) {
        std::cout << std::left  << std::setw(10) << p.name
                  << std::right << std::fixed
                  << std::setw(12) << std::setprecision(4)
                  << p.planet->orbit.semi_major_axis_au
                  << std::setw(10) << std::setprecision(5)
                  << p.planet->orbit.eccentricity
                  << std::setw(12) << std::setprecision(0)
                  << p.planet->radius_km << "\n";
    }

    // -------------------------------------------------------------------------
    // Earth-Mars distance via heliocentric coordinates
    // -------------------------------------------------------------------------
    std::cout << "\n--- Earth-Mars Distance (J2000.0) ---\n";
    const double ex = earth_h.x().value(), ey = earth_h.y().value(), ez = earth_h.z().value();
    const double mx = mars_h.x().value(),  my = mars_h.y().value(),  mz = mars_h.z().value();
    const double dist_au = std::sqrt((ex-mx)*(ex-mx) + (ey-my)*(ey-my) + (ez-mz)*(ez-mz));
    std::cout << "Distance: " << std::setprecision(4) << dist_au << " AU"
              << "  (" << std::setprecision(0) << dist_au * 1.496e8 << " km)\n";

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
