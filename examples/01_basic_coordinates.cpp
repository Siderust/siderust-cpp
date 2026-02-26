/**
 * @file 01_basic_coordinates.cpp
 * @brief C++ port of siderust/examples/01_basic_coordinates.rs
 */

#include <iostream>
#include <iomanip>

#include <siderust/siderust.hpp>

int main() {
  using namespace siderust;
  using namespace qtty::literals;

  std::cout << "=== 01_basic_coordinates (C++) ===\n";

  // Time
  const JulianDate jd = JulianDate::J2000();

  // ==========================================================================
  // Cartesian coordinates (heliocentric example)
  // ==========================================================================
  auto earth = ephemeris::earth_heliocentric(jd);
  std::cout << "Earth heliocentric (EclipticMeanJ2000):\n";
  std::cout << "  X = " << earth.x() << "\n";
  std::cout << "  Y = " << earth.y() << "\n";
  std::cout << "  Z = " << earth.z() << "\n";
  std::cout << "  Distance = " << earth.distance() << "\n\n";

  // ==========================================================================
  // Spherical direction and frame conversions
  // ==========================================================================
  const Geodetic site(-17.8947_deg, 28.7606_deg, 2396.0_m);

  spherical::direction::ICRS vega_icrs(279.23473_deg, 38.78369_deg);
  auto vega_ecl = vega_icrs.to_frame<frames::EclipticMeanJ2000>(jd);
  auto vega_true = vega_icrs.to_frame<frames::EquatorialTrueOfDate>(jd);
  auto vega_horiz = vega_icrs.to_horizontal(jd, site);

  std::cout << "Direction transforms:\n";
  std::cout << "  ICRS RA/Dec: " << vega_icrs << "\n";
  std::cout << "  Ecliptic lon/lat: " << vega_ecl << "\n";
  std::cout << "  True-of-date RA/Dec: " << vega_true << "\n";
  std::cout << "  Horizontal az/alt: " << vega_horiz << "\n\n";

  // ==========================================================================
  // Directions <-> Positions
  // ==========================================================================
  spherical::position::ICRS<qtty::AstronomicalUnit> synthetic_star(210.0_deg, -12.0_deg, 4.2_au);
  std::cout << "Typed positions:\n";
  std::cout << "  Synthetic star distance: " << synthetic_star.distance() << "\n";

  // ==========================================================================
  // Type safety demonstration
  // ==========================================================================
  const auto ecef_m = site.to_cartesian();
  static_assert(std::is_same_v<decltype(ecef_m), cartesian::position::ECEF<qtty::Meter>>);
  std::cout << "Geodetic -> ECEF: " << site << " -> " << ecef_m << "\n";

  std::cout << "=== Example Complete ===\n";
  return 0;
}
