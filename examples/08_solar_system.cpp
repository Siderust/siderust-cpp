// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// @file 08_solar_system.cpp
/// @brief Solar System + Planets Module Tour.
///
/// Demonstrates:
/// - Planet constants (mass, radius, orbit) and orbital period via Kepler's 3rd law
/// - VSOP87 heliocentric + barycentric ephemerides for all planets, Sun, and Moon
/// - Center transforms (heliocentric → geocentric)
/// - Moon geocentric position (ELP2000)
/// - Custom planet construction
/// - Current-epoch snapshot using system time
///
/// Build & run:
///   cmake --build build-local --target 08_solar_system_example
///   ./build-local/08_solar_system_example

#include <siderust/siderust.hpp>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>

using namespace siderust;
using namespace siderust::frames;
using namespace siderust::centers;
using namespace qtty::literals;

// ─── Kepler's 3rd law: compute orbital period from semi-major axis ──────────

/// GM_Sun in AU³/day².  From IAU: k² = μ where k = 0.01720209895 AU^(3/2)/day.
static constexpr double GM_SUN_AU3_DAY2 = 0.01720209895 * 0.01720209895;

/// Sidereal period via Kepler's 3rd law: T = 2π √(a³/μ) [days].
inline qtty::Day orbit_period(const Orbit &orb) {
    double a = orb.semi_major_axis.value();
    double T = 2.0 * M_PI * std::sqrt(a * a * a / GM_SUN_AU3_DAY2);
    return qtty::Day(T);
}

// ─── JulianDate from system clock ───────────────────────────────────────────

/// Approximate JulianDate::now() using <chrono>.
/// JD of the Unix epoch (1970-01-01T00:00:00 UTC) = 2440587.5
inline JulianDate jd_now() {
    using namespace std::chrono;
    auto unix_sec = duration_cast<milliseconds>(
                        system_clock::now().time_since_epoch())
                        .count() /
                    1000.0;
    return JulianDate(2440587.5 + unix_sec / 86400.0);
}

// ─── Sections ───────────────────────────────────────────────────────────────

void section_planet_constants_and_periods() {
    std::puts("1) PLANET CONSTANTS + ORBITAL PERIOD (Kepler 3rd law)\n");

    struct PlanetInfo {
        const char *name;
        const Planet *planet;
    };
    const PlanetInfo planets[] = {
        {"Mercury", &MERCURY}, {"Venus", &VENUS},   {"Earth", &EARTH},
        {"Mars", &MARS},       {"Jupiter", &JUPITER}, {"Saturn", &SATURN},
        {"Uranus", &URANUS},   {"Neptune", &NEPTUNE},
    };

    std::printf("%-8s %10s %10s %10s\n", "Planet", "a [AU]", "e", "Period");
    std::puts("------------------------------------------------");
    for (auto &[name, p] : planets) {
        auto period = orbit_period(p->orbit);
        std::printf("%-8s %10.6f %10.6f ", name,
                    p->orbit.semi_major_axis.value(), p->orbit.eccentricity);
        std::cout << std::fixed << std::setprecision(2) << period << std::endl;
    }
    std::puts("");
}

void section_vsop87_positions(const JulianDate &jd) {
    std::puts("2) VSOP87 EPHEMERIDES (HELIOCENTRIC + BARYCENTRIC)");
    std::puts("-----------------------------------------------");

    auto earth_h = ephemeris::earth_heliocentric(jd);
    auto mars_h  = ephemeris::mars_heliocentric(jd);
    auto earth_mars = earth_h.distance_to(mars_h);

    std::cout << "Earth heliocentric distance: " << std::fixed
              << std::setprecision(6) << earth_h.distance() << std::endl;
    std::cout << "Mars heliocentric distance:  " << std::fixed
              << std::setprecision(6) << mars_h.distance() << std::endl;
    std::cout << "Earth-Mars separation:       " << std::fixed
              << std::setprecision(6) << earth_mars
              << " (" << std::fixed << std::setprecision(0)
              << earth_mars.to<qtty::Kilometer>() << ")" << std::endl;

    auto sun_bary = ephemeris::sun_barycentric(jd);
    std::cout << "Sun barycentric offset from SSB: " << std::fixed
              << std::setprecision(8) << sun_bary.distance() << std::endl;

    auto jupiter_bary = ephemeris::jupiter_barycentric(jd);
    std::cout << "\nJupiter barycentric position at J2000:" << std::endl;
    std::cout << "  x = " << std::fixed << std::setprecision(6)
              << jupiter_bary.x() << std::endl;
    std::cout << "  y = " << std::fixed << std::setprecision(6)
              << jupiter_bary.y() << std::endl;
    std::cout << "  z = " << std::fixed << std::setprecision(6)
              << jupiter_bary.z() << std::endl;
    std::puts("");
}

void section_center_transforms(const JulianDate &jd) {
    std::puts("3) CENTER TRANSFORMS (HELIOCENTRIC -> GEOCENTRIC)");
    std::puts("-----------------------------------------------");

    auto mars_helio = ephemeris::mars_heliocentric(jd);
    auto mars_geo   = mars_helio.to_center<Geocentric>(jd);

    std::cout << "Mars geocentric distance at J2000: " << std::fixed
              << std::setprecision(6) << mars_geo.distance() << std::endl;
    std::cout << "Mars geocentric distance at J2000: " << std::fixed
              << std::setprecision(0) << mars_geo.distance().to<qtty::Kilometer>()
              << std::endl;
    std::puts("");
}

void section_moon(const JulianDate &jd) {
    std::puts("4) MOON (ELP2000)");
    std::puts("-----------------");

    auto moon_geo = ephemeris::moon_geocentric(jd);
    std::cout << "Moon geocentric distance (ELP2000): " << std::fixed
              << std::setprecision(1) << moon_geo.distance()
              << " (" << std::setprecision(6)
              << moon_geo.distance().to<qtty::AstronomicalUnit>() << ")"
              << std::endl;
    std::puts("");
}

void section_trait_dispatch(const JulianDate &jd) {
    std::puts("5) EPHEMERIS DISPATCH (all inner planets)");
    std::puts("-----------------------------------------");

    // In C++ there is no dynamic trait dispatch for VSOP87.
    // Instead, we call each planet's ephemeris directly.
    struct PlanetEphem {
        const char *name;
        decltype(ephemeris::mercury_heliocentric) *helio;
        decltype(ephemeris::mercury_barycentric) *bary;
    };

    const PlanetEphem planets[] = {
        {"Mercury", &ephemeris::mercury_heliocentric, &ephemeris::mercury_barycentric},
        {"Venus",   &ephemeris::venus_heliocentric,   &ephemeris::venus_barycentric},
        {"Earth",   &ephemeris::earth_heliocentric,    &ephemeris::earth_barycentric},
        {"Mars",    &ephemeris::mars_heliocentric,     &ephemeris::mars_barycentric},
    };

    for (auto &[name, helio_fn, bary_fn] : planets) {
        auto helio = helio_fn(jd);
        auto bary  = bary_fn(jd);
        std::cout << std::left << std::setw(8) << name
                  << " helio=" << std::fixed << std::setprecision(5)
                  << helio.distance()
                  << "  bary=" << bary.distance() << std::endl;
    }
    std::puts("");
}

void section_custom_planet() {
    std::puts("6) CUSTOM PLANET + ORBITAL PERIOD");
    std::puts("---------------------------------");

    Planet demo_world{
        qtty::Kilogram(5.972e24 * 2.0), // mass: double the Earth
        qtty::Kilometer(6371.0 * 1.3),  // radius: 30% bigger
        Orbit{1.4_au, 0.07, 4.0_deg,
              120.0_deg, 80.0_deg, 10.0_deg,
              JulianDate::J2000().value()}
    };

    auto period = orbit_period(demo_world.orbit);

    std::puts("Custom planet built at runtime:");
    std::cout << "  mass   = " << std::scientific << std::setprecision(3)
              << demo_world.mass << std::endl;
    std::cout << "  radius = " << std::fixed << std::setprecision(1)
              << demo_world.radius << std::endl;
    std::cout << "  a      = " << std::setprecision(6)
              << demo_world.orbit.semi_major_axis << std::endl;
    std::cout << "  sidereal period = " << std::fixed << std::setprecision(2)
              << period << "\n" << std::endl;
}

void section_current_snapshot(const JulianDate &now) {
    std::puts("7) CURRENT SNAPSHOT");
    std::puts("-------------------");

    auto earth_now = ephemeris::earth_heliocentric(now);
    auto mars_now  = ephemeris::mars_heliocentric(now);
    auto mars_geo_now = mars_now.to_center<Geocentric>(now);

    std::cout << "Earth-Sun distance now: " << std::fixed
              << std::setprecision(6) << earth_now.distance() << std::endl;
    std::cout << "Mars-Sun distance now:  " << std::fixed
              << std::setprecision(6) << mars_now.distance() << std::endl;
    std::cout << "Mars-Earth distance now: " << std::fixed
              << std::setprecision(6) << mars_geo_now.distance()
              << " (" << std::setprecision(0)
              << mars_geo_now.distance().to<qtty::Kilometer>() << ")"
              << std::endl;

    std::puts("\n=== End of example ===");
}

// ─── main ─────────────────────────────────────────────────────────────────────

int main() {
    JulianDate jd = JulianDate::J2000();
    JulianDate now = jd_now();

    std::puts("=== Siderust Solar System Module Tour ===\n");
    std::cout << "Epoch used for deterministic outputs: J2000 (JD " << std::fixed
              << std::setprecision(1) << jd << ")" << std::endl;
    std::cout << "Current epoch snapshot: JD " << std::setprecision(6)
              << now << "\n" << std::endl;

    section_planet_constants_and_periods();
    section_vsop87_positions(jd);
    section_center_transforms(jd);
    section_moon(jd);
    section_trait_dispatch(jd);
    section_custom_planet();
    section_current_snapshot(now);

    return 0;
}
