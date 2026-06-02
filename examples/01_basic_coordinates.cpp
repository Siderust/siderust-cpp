// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// Basic Coordinates Example
///
/// Build with: cmake --build build --target 01_basic_coordinates_example
/// Run with:   ./build/01_basic_coordinates_example

#include <siderust/siderust.hpp>

#include <iostream>

using namespace siderust;
using namespace siderust::frames;
using namespace siderust::centers;
using namespace qtty::literals;

int main() {
  std::cout << "=== Siderust Basic Coordinates Example ===\n\n";

  cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit> earth(1.0, 0.0, 0.0);
  std::cout << "Earth heliocentric ecliptic: " << earth << '\n';
  std::cout << "  distance from Sun: " << earth.distance() << "\n\n";

  cartesian::Position<Geocentric, EquatorialMeanJ2000, qtty::Kilometer> moon(300000.0, 200000.0,
                                                                             100000.0);
  std::cout << "Moon geocentric equatorial: " << moon << '\n';
  std::cout << "  distance from Earth: " << moon.distance() << "\n\n";

  spherical::direction::EquatorialMeanJ2000 polaris(37.95_deg, 89.26_deg);
  std::cout << "Polaris direction: " << polaris << "\n\n";

  auto betelgeuse_distance = qtty::LightYear(500.0).to<qtty::AstronomicalUnit>();
  spherical::position::ICRS<qtty::AstronomicalUnit> betelgeuse(88.79_deg, 7.41_deg,
                                                               betelgeuse_distance);
  std::cout << "Betelgeuse: " << betelgeuse << "\n\n";

  spherical::direction::Horizontal zenith(0.0_deg, 90.0_deg);
  std::cout << "Zenith (horizontal): " << zenith << '\n';
  spherical::Position<Geocentric, Horizontal, qtty::Kilometer> cloud(zenith.az(), zenith.alt(),
                                                                     5000.0_km);
  std::cout << "Cloud at zenith: " << cloud << "\n\n";

  cartesian::Position<Geocentric, EquatorialMeanJ2000, qtty::AstronomicalUnit> cart_pos(0.5, 0.5,
                                                                                        0.707);
  std::cout << "Cartesian: " << cart_pos << '\n';
  auto sph_pos = cart_pos.to_spherical();
  std::cout << "Spherical:  " << sph_pos << '\n';
  std::cout << "Back to cartesian: " << sph_pos.to_cartesian() << "\n\n";

  cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit> helio(1.0, 0.0, 0.0);
  cartesian::Position<Geocentric, EquatorialMeanJ2000, qtty::AstronomicalUnit> geo(0.0, 1.0, 0.0);
  std::cout << "Different centers cannot be subtracted directly:\n";
  std::cout << "  " << helio << '\n';
  std::cout << "  " << geo << '\n';
  auto sep = helio.distance_to(
      cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>(1.5, 0.0, 0.0));
  std::cout << "Separation (same type): " << sep << "\n\n";

  std::cout << "Centers: Barycentric, Heliocentric, Geocentric, Topocentric, Bodycentric\n";
  std::cout << "Frames:  EclipticMeanJ2000, EquatorialMeanJ2000, Horizontal, ICRS, ECEF\n";
  return 0;
}
