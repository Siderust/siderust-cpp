// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file bodycentric_coordinates.cpp
 * @brief Body-Centric Coordinates Example (mirrors Rust's bodycentric_coordinates.rs)
 *
 * This example demonstrates body-centric coordinate transforms: viewing
 * positions from arbitrary orbiting bodies (satellites, planets, moons).
 *
 * Key API:
 *  - `BodycentricParams::geocentric(orbit)`  / `::heliocentric(orbit)` — params
 *  - `to_bodycentric(pos, params, jd)`       — transform to body frame
 *  - `BodycentricPos<F,U>::to_geocentric(jd)` — inverse transform
 *  - `kepler_position<C>(orbit, jd)`         — Keplerian propagator
 */

#include <siderust/siderust.hpp>

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace siderust;
using namespace siderust::frames;
using namespace siderust::centers;
using qtty::AstronomicalUnit;

// ---------------------------------------------------------------------------
// Distances helpers
// ---------------------------------------------------------------------------
static double au_magnitude(const cartesian::Position<Geocentric, EclipticMeanJ2000, AstronomicalUnit> &p) {
  double x = p.x().value(), y = p.y().value(), z = p.z().value();
  return std::sqrt(x * x + y * y + z * z);
}
template <typename F, typename U>
static double magnitude(const BodycentricPos<F, U> &p) {
  double x = p.x().value(), y = p.y().value(), z = p.z().value();
  return std::sqrt(x * x + y * y + z * z);
}
template <typename C, typename F, typename U>
static double pos_magnitude(const cartesian::Position<C, F, U> &p) {
  double x = p.x().value(), y = p.y().value(), z = p.z().value();
  return std::sqrt(x * x + y * y + z * z);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main() {
  std::cout << "=== Body-Centric Coordinates Example ===\n\n";

  // J2000.0
  const JulianDate jd(2451545.0);
  std::cout << "Reference time: J2000.0 (JD " << std::fixed
            << std::setprecision(1) << jd.value() << ")\n\n";

  // =========================================================================
  // 1. Satellite-Centric Coordinates (ISS example)
  // =========================================================================
  std::cout << "1. SATELLITE-CENTRIC COORDINATES\n";
  std::cout << "--------------------------------\n";

  // ISS-like orbit: ~6 378 km = 0.0000426 AU above Earth
  constexpr double KM_PER_AU = 149'597'870.7;
  constexpr double ISS_ALTITUDE_KM = 6'378.0;  // Earth radius ≈ altitude
  constexpr double ISS_SMA_AU = ISS_ALTITUDE_KM / KM_PER_AU;

  Orbit iss_orbit{ISS_SMA_AU, 0.001, 51.6, 0.0, 0.0, 0.0, jd.value()};

  BodycentricParams iss_params = BodycentricParams::geocentric(iss_orbit);

  // ISS position (geocentric) via Keplerian propagation
  auto iss_pos = kepler_position<Geocentric>(iss_orbit, jd);
  std::cout << "ISS orbit:\n";
  std::cout << "  Semi-major axis : " << std::setprecision(8) << ISS_SMA_AU
            << " AU (" << ISS_ALTITUDE_KM << " km)\n";
  std::cout << "  Eccentricity    : " << iss_orbit.eccentricity << "\n";
  std::cout << "  Inclination     : " << iss_orbit.inclination_deg << "°\n";
  std::cout << std::setprecision(8);
  std::cout << "ISS position (Geocentric EclipticMeanJ2000):\n";
  std::cout << "  X = " << iss_pos.x().value() << " AU\n";
  std::cout << "  Y = " << iss_pos.y().value() << " AU\n";
  std::cout << "  Z = " << iss_pos.z().value() << " AU\n";
  std::cout << "  Distance from Earth: " << iss_pos.distance().value() << " AU ("
            << iss_pos.distance().value() * KM_PER_AU << " km)\n\n";

  // Moon's approximate geocentric position (~384 400 km = 0.00257 AU)
  cartesian::Position<Geocentric, EclipticMeanJ2000, AstronomicalUnit>
      moon_geo(0.00257, 0.0, 0.0);
  std::cout << "Moon position (Geocentric):\n";
  std::cout << "  Distance from Earth: " << moon_geo.distance().value() << " AU ("
            << moon_geo.distance().value() * KM_PER_AU << " km)\n\n";

  // Transform to ISS-centric
  auto moon_from_iss = to_bodycentric(moon_geo, iss_params, jd);
  std::cout << "Moon as seen from ISS:\n";
  std::cout << "  X = " << moon_from_iss.x().value() << " AU\n";
  std::cout << "  Y = " << moon_from_iss.y().value() << " AU\n";
  std::cout << "  Z = " << moon_from_iss.z().value() << " AU\n";
  std::cout << "  Distance from ISS: " << moon_from_iss.distance().value() << " AU ("
            << moon_from_iss.distance().value() * KM_PER_AU << " km)\n\n";

  // =========================================================================
  // 2. Mars-Centric Coordinates
  // =========================================================================
  std::cout << "2. MARS-CENTRIC COORDINATES\n";
  std::cout << "---------------------------\n";

  Orbit mars_orbit{1.524, 0.0934, 1.85, 49.56, 286.5, 19.41, jd.value()};
  BodycentricParams mars_params = BodycentricParams::heliocentric(mars_orbit);

  auto earth_helio = ephemeris::earth_heliocentric(jd);
  auto mars_helio  = ephemeris::mars_heliocentric(jd);

  std::cout << "Earth (Heliocentric): distance from Sun = "
            << earth_helio.distance().value() << " AU\n";
  std::cout << "Mars  (Heliocentric): distance from Sun = "
            << mars_helio.distance().value() << " AU\n\n";

  auto earth_from_mars = to_bodycentric(earth_helio, mars_params, jd);
  std::cout << "Earth as seen from Mars:\n";
  std::cout << "  X = " << earth_from_mars.x().value() << " AU\n";
  std::cout << "  Y = " << earth_from_mars.y().value() << " AU\n";
  std::cout << "  Z = " << earth_from_mars.z().value() << " AU\n";
  std::cout << "  Distance from Mars: " << earth_from_mars.distance().value() << " AU\n\n";

  // =========================================================================
  // 3. Venus-Centric Coordinates
  // =========================================================================
  std::cout << "3. VENUS-CENTRIC COORDINATES\n";
  std::cout << "----------------------------\n";

  Orbit venus_orbit{0.723, 0.0067, 3.39, 76.68, 131.53, 50.42, jd.value()};
  BodycentricParams venus_params = BodycentricParams::heliocentric(venus_orbit);

  auto venus_helio = ephemeris::venus_heliocentric(jd);
  std::cout << "Venus (Heliocentric): distance from Sun = "
            << venus_helio.distance().value() << " AU\n\n";

  auto earth_from_venus = to_bodycentric(earth_helio, venus_params, jd);
  std::cout << "Earth as seen from Venus:\n";
  std::cout << "  Distance: " << earth_from_venus.distance().value() << " AU\n\n";

  auto mars_from_venus = to_bodycentric(mars_helio, venus_params, jd);
  std::cout << "Mars as seen from Venus:\n";
  std::cout << "  Distance: " << mars_from_venus.distance().value() << " AU\n\n";

  // =========================================================================
  // 4. Round-Trip Transformation
  // =========================================================================
  std::cout << "4. ROUND-TRIP TRANSFORMATION\n";
  std::cout << "----------------------------\n";

  // Start from a known geocentric position
  cartesian::Position<Geocentric, EclipticMeanJ2000, AstronomicalUnit>
      original_pos(0.001, 0.002, 0.003);
  std::cout << "Original position (Geocentric):\n";
  std::cout << "  X = " << std::setprecision(12) << original_pos.x().value()
            << " AU\n";
  std::cout << "  Y = " << original_pos.y().value() << " AU\n";
  std::cout << "  Z = " << original_pos.z().value() << " AU\n\n";

  // To Mars-centric and back to geocentric
  auto mars_centric = to_bodycentric(original_pos, mars_params, jd);
  std::cout << "Transformed to Mars-centric:\n";
  std::cout << "  Distance from Mars: " << std::setprecision(8)
            << mars_centric.distance().value() << " AU\n\n";

  auto recovered = mars_centric.to_geocentric(jd);
  std::cout << "Recovered position (Geocentric):\n";
  std::cout << "  X = " << std::setprecision(12) << recovered.x().value()
            << " AU\n";
  std::cout << "  Y = " << recovered.y().value() << " AU\n";
  std::cout << "  Z = " << recovered.z().value() << " AU\n\n";

  double dx = original_pos.x().value() - recovered.x().value();
  double dy = original_pos.y().value() - recovered.y().value();
  double dz = original_pos.z().value() - recovered.z().value();
  double diff = std::sqrt(dx * dx + dy * dy + dz * dz);
  std::cout << "Total difference: " << diff
            << " AU (should be ~0 within floating-point precision)\n\n";

  // =========================================================================
  // 5. Directions as Free Vectors
  // =========================================================================
  std::cout << "5. DIRECTIONS AS FREE VECTORS\n";
  std::cout << "------------------------------\n";

  cartesian::Direction<EquatorialMeanJ2000> star_dir(0.707, 0.0, 0.707);
  std::cout << "Star direction (EquatorialMeanJ2000):\n";
  std::cout << "  X = " << std::setprecision(3) << star_dir.x << "\n";
  std::cout << "  Y = " << star_dir.y << "\n";
  std::cout << "  Z = " << star_dir.z << "\n\n";

  std::cout << "Note: Directions are free vectors — they represent 'which way'\n"
               "without reference to any origin. A distant star appears in the\n"
               "same direction from Earth or from the ISS.\n\n";

  // =========================================================================
  std::cout << "=== Example Complete ===\n\n";
  std::cout << "Key Takeaways:\n";
  std::cout << "- Body-centric coordinates work for any orbiting body\n";
  std::cout << "- Satellite-centric: use BodycentricParams::geocentric()\n";
  std::cout << "- Planet-centric:    use BodycentricParams::heliocentric()\n";
  std::cout << "- Directions are free vectors (no center, only frame)\n";
  std::cout << "- Round-trip transformations preserve positions within floating-point precision\n";

  return 0;
}
