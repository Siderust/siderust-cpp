// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// @file 08_solar_system.cpp
/// @brief Solar System + Planets Module Tour.
///
/// Demonstrates:
/// - Planet constants (mass, radius, orbit) and orbital period via Kepler's 3rd
/// law
/// - VSOP87 heliocentric + barycentric ephemerides for all planets, Sun, and
/// Moon
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
inline qtty::Day orbit_period(const KeplerianOrbit &orb) {
  double a = orb.semi_major_axis.value();
  double T = 2.0 * constants::pi * std::sqrt(a * a * a / GM_SUN_AU3_DAY2);
  return qtty::Day(T);
}

// ─── Time<TT, JD> from system clock ───────────────────────────────────────────

/// Approximate current TT Julian Date using the Unix encoding bridge.
inline Time<TT, JD> jd_now() {
  using namespace std::chrono;
  auto unix_sec =
      duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() / 1000.0;
  return Time<UTC, Unix>(unix_sec).to<scale::TT>().to<format::JD>();
}

// ─── Sections ───────────────────────────────────────────────────────────────

void section_planet_constants_and_periods() {
  std::cout << "1) PLANET CONSTANTS + ORBITAL PERIOD (Kepler 3rd law)\n\n";

  struct PlanetInfo {
    const char *name;
    const Planet *planet;
  };
  const PlanetInfo planets[] = {
      {"Mercury", &MERCURY()}, {"Venus", &VENUS()},     {"Earth", &EARTH()},
      {"Mars", &MARS()},       {"Jupiter", &JUPITER()}, {"Saturn", &SATURN()},
      {"Uranus", &URANUS()},   {"Neptune", &NEPTUNE()},
  };

  std::cout << std::left << std::setw(8) << "Planet" << std::right << std::setw(10) << "a [AU]"
            << std::setw(10) << "e" << std::setw(10) << "Period<TT, MJD>" << '\n';
  std::cout << std::string(38, '-') << '\n';
  for (auto &[name, p] : planets) {
    auto period = orbit_period(p->orbit);
    std::cout << std::left << std::setw(8) << name << std::right << std::fixed
              << std::setprecision(6) << std::setw(10) << p->orbit.semi_major_axis.value()
              << std::setw(10) << p->orbit.eccentricity.value << std::setw(10)
              << std::setprecision(2) << period << '\n';
  }
  std::cout << '\n';
}

void section_vsop87_positions(const Time<TT, JD> &jd) {
  std::cout << "2) VSOP87 EPHEMERIDES (HELIOCENTRIC + BARYCENTRIC)\n"
            << "-----------------------------------------------\n";

  auto earth_h = ephemeris::earth_heliocentric(jd);
  auto mars_h = ephemeris::mars_heliocentric(jd);
  auto earth_mars = earth_h.distance_to(mars_h);

  std::cout << "Earth heliocentric distance: " << std::fixed << std::setprecision(6)
            << earth_h.distance() << std::endl;
  std::cout << "Mars heliocentric distance:  " << std::fixed << std::setprecision(6)
            << mars_h.distance() << std::endl;
  std::cout << "Earth-Mars separation:       " << std::fixed << std::setprecision(6) << earth_mars
            << " (" << std::fixed << std::setprecision(0) << earth_mars.to<qtty::Kilometer>() << ")"
            << std::endl;

  auto sun_bary = ephemeris::sun_barycentric(jd);
  std::cout << "Sun barycentric offset from SSB: " << std::fixed << std::setprecision(8)
            << sun_bary.distance() << std::endl;

  auto jupiter_bary = ephemeris::jupiter_barycentric(jd);
  std::cout << "\nJupiter barycentric position at J2000:\n  " << jupiter_bary << "\n\n";
}

void section_center_transforms(const Time<TT, JD> &jd) {
  std::cout << "3) CENTER TRANSFORMS (HELIOCENTRIC -> GEOCENTRIC)\n"
            << "-----------------------------------------------\n";

  auto mars_helio = ephemeris::mars_heliocentric(jd);
  auto mars_geo = mars_helio.to_center<Geocentric>(jd);

  std::cout << "Mars geocentric distance at J2000: " << std::fixed << std::setprecision(6)
            << mars_geo.distance() << std::endl;
  std::cout << "Mars geocentric distance at J2000: " << std::fixed << std::setprecision(0)
            << mars_geo.distance().to<qtty::Kilometer>() << std::endl;
  std::cout << '\n';
}

void section_moon(const Time<TT, JD> &jd) {
  std::cout << "4) MOON (ELP2000)\n"
            << "-----------------\n";

  auto moon_geo = ephemeris::moon_geocentric(jd);
  std::cout << "Moon geocentric distance (ELP2000): " << std::fixed << std::setprecision(1)
            << moon_geo.distance() << " (" << std::setprecision(6)
            << moon_geo.distance().to<qtty::AstronomicalUnit>() << ")" << std::endl;
  std::cout << '\n';
}

void section_trait_dispatch(const Time<TT, JD> &jd) {
  std::cout << "5) EPHEMERIS DISPATCH (all inner planets)\n"
            << "-----------------------------------------\n";

  // In C++ there is no dynamic trait dispatch for VSOP87.
  // Instead, we call each planet's ephemeris directly.
  struct PlanetEphem {
    const char *name;
    decltype(ephemeris::mercury_heliocentric) *helio;
    decltype(ephemeris::mercury_barycentric) *bary;
  };

  const PlanetEphem planets[] = {
      {"Mercury", &ephemeris::mercury_heliocentric, &ephemeris::mercury_barycentric},
      {"Venus", &ephemeris::venus_heliocentric, &ephemeris::venus_barycentric},
      {"Earth", &ephemeris::earth_heliocentric, &ephemeris::earth_barycentric},
      {"Mars", &ephemeris::mars_heliocentric, &ephemeris::mars_barycentric},
  };

  for (auto &[name, helio_fn, bary_fn] : planets) {
    auto helio = helio_fn(jd);
    auto bary = bary_fn(jd);
    std::cout << std::left << std::setw(8) << name << " helio=" << std::fixed
              << std::setprecision(5) << helio.distance() << "  bary=" << bary.distance()
              << std::endl;
  }
  std::cout << '\n';
}

void section_custom_planet() {
  std::cout << "6) CUSTOM PLANET + ORBITAL PERIOD\n"
            << "---------------------------------\n";

  Planet demo_world{qtty::Kilogram(5.972e24 * 2.0), // mass: double the Earth
                    qtty::Kilometer(6371.0 * 1.3),  // radius: 30% bigger
                    KeplerianOrbit{1.4_au, Eccentricity{0.07}, 4.0_deg, 120.0_deg, 80.0_deg,
                                   10.0_deg, Time<TT, JD>::J2000()}};

  auto period = orbit_period(demo_world.orbit);

  std::cout << "Custom planet built at runtime:\n";
  std::cout << "  mass   = " << std::scientific << std::setprecision(3) << demo_world.mass
            << std::endl;
  std::cout << "  radius = " << std::fixed << std::setprecision(1) << demo_world.radius
            << std::endl;
  std::cout << "  a      = " << std::setprecision(6) << demo_world.orbit.semi_major_axis
            << std::endl;
  std::cout << "  sidereal period = " << std::fixed << std::setprecision(2) << period << "\n"
            << std::endl;
}

void section_current_snapshot(const Time<TT, JD> &now) {
  std::cout << "7) CURRENT SNAPSHOT\n"
            << "-------------------\n";

  auto earth_now = ephemeris::earth_heliocentric(now);
  auto mars_now = ephemeris::mars_heliocentric(now);
  auto mars_geo_now = mars_now.to_center<Geocentric>(now);

  std::cout << "Earth-Sun distance now: " << std::fixed << std::setprecision(6)
            << earth_now.distance() << std::endl;
  std::cout << "Mars-Sun distance now:  " << std::fixed << std::setprecision(6)
            << mars_now.distance() << std::endl;
  std::cout << "Mars-Earth distance now: " << std::fixed << std::setprecision(6)
            << mars_geo_now.distance() << " (" << std::setprecision(0)
            << mars_geo_now.distance().to<qtty::Kilometer>() << ")" << std::endl;

  std::cout << "\n=== End of example ===\n";
}

// ─── main
// ─────────────────────────────────────────────────────────────────────

int main() {
  auto jd = Time<TT, JD>::J2000();
  auto now = jd_now();

  std::cout << "=== Siderust Solar System Module Tour ===\n\n";
  std::cout << "Epoch used for deterministic outputs: J2000 (JD " << std::fixed
            << std::setprecision(1) << jd << ")" << std::endl;
  std::cout << "Current epoch snapshot: JD " << std::setprecision(6) << now << "\n" << std::endl;

  section_planet_constants_and_periods();
  section_vsop87_positions(jd);
  section_center_transforms(jd);
  section_moon(jd);
  section_trait_dispatch(jd);
  section_custom_planet();
  section_current_snapshot(now);

  return 0;
}
