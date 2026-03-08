// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// Coordinate Operations Example
///
/// Demonstrates what you can do with spherical/cartesian coordinate types:
/// angular separation (Vincenty formula), cross-validation against the
/// cartesian dot product, Euclidean 3D distance, and how type safety prevents
/// accidentally mixing incompatible coordinate systems.
///
/// Build with: cmake --build build-local --target
/// 13_coordinate_operations_example Run with:
/// ./build-local/13_coordinate_operations_example

#include <siderust/siderust.hpp>

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace siderust;
using namespace siderust::frames;
using namespace siderust::centers;
using namespace qtty::literals;

int main() {
  std::cout << std::fixed;
  std::cout << "=== Siderust Coordinate Operations Example ===" << std::endl
            << std::endl;

  // =========================================================================
  // 1. Angular Separation — Spherical Directions
  // =========================================================================
  std::cout << "1. ANGULAR SEPARATION (SPHERICAL DIRECTIONS)" << std::endl;
  std::cout << "---------------------------------------------" << std::endl;

  // Two well-known stars using EquatorialMeanJ2000 directions.
  // Constructor: Direction(azimuth, polar) = Direction(RA, Dec)
  spherical::direction::EquatorialMeanJ2000 polaris(
      37.9546_deg, // Right Ascension
      89.2641_deg  // Declination
  );
  spherical::direction::EquatorialMeanJ2000 sirius(
      101.2872_deg, // Right Ascension
      -16.7161_deg  // Declination
  );

  auto sep = polaris.angular_separation(sirius);
  std::cout << std::setprecision(4);
  std::cout << "Polaris  (RA=" << polaris.ra() << ", Dec=" << polaris.dec()
            << ")" << std::endl;
  std::cout << "Sirius   (RA=" << sirius.ra() << ", Dec=" << sirius.dec() << ")"
            << std::endl;
  std::cout << "  Angular separation = " << sep << std::endl << std::endl;

  // Nearby pair — separation should be about 0.66°
  spherical::direction::EquatorialMeanJ2000 star_a(10.0_deg, 30.0_deg);
  spherical::direction::EquatorialMeanJ2000 star_b(10.5_deg, 30.5_deg);
  auto close_sep = star_a.angular_separation(star_b);
  std::cout << "Nearby pair (0.5\u00b0 apart in both RA and Dec):" << std::endl;
  std::cout << "  Angular separation = " << close_sep << std::endl << std::endl;

  // Self-separation must be exactly 0
  auto self_sep = polaris.angular_separation(polaris);
  std::cout << std::setprecision(6);
  std::cout << "Self-separation of Polaris = " << self_sep << "  (must be 0)"
            << std::endl
            << std::endl;

  // =========================================================================
  // 2. Cross-Validation: Spherical Vincenty vs. Cartesian Dot Product
  // =========================================================================
  std::cout << "2. CROSS-VALIDATION: SPHERICAL vs. CARTESIAN" << std::endl;
  std::cout << "---------------------------------------------" << std::endl;

  // Convert spherical directions to Cartesian unit vectors
  auto polaris_cart = polaris.to_cartesian();
  auto sirius_cart = sirius.to_cartesian();

  // angle_to returns radians
  double angle_rad = polaris_cart.angle_to(sirius_cart);
  double angle_deg = angle_rad * 180.0 / M_PI;

  std::cout << std::setprecision(4);
  std::cout << "Polaris cartesian: (" << polaris_cart.x << ", "
            << polaris_cart.y << ", " << polaris_cart.z << ")" << std::endl;
  std::cout << "Sirius  cartesian: (" << sirius_cart.x << ", " << sirius_cart.y
            << ", " << sirius_cart.z << ")" << std::endl;
  std::cout << std::setprecision(6);
  std::cout << "  angle_to (Cartesian)          = " << angle_rad
            << " rad = " << std::setprecision(4) << angle_deg << "\u00b0"
            << std::endl;
  std::cout << "  angular_separation (Vincenty)  = " << sep << std::endl;
  std::cout << std::scientific << std::setprecision(2);
  std::cout << "  Difference                     = "
            << std::abs(angle_deg - sep.value())
            << "\u00b0  (near machine epsilon)" << std::endl
            << std::endl;

  // =========================================================================
  // 3. Dot Product and Perpendicularity
  // =========================================================================
  std::cout << std::fixed;
  std::cout << "3. DOT PRODUCT" << std::endl;
  std::cout << "--------------" << std::endl;

  // North celestial pole and a point on the equator are perpendicular
  spherical::direction::EquatorialMeanJ2000 north_pole_dir(0.0_deg, 90.0_deg);
  spherical::direction::EquatorialMeanJ2000 equatorial_dir(0.0_deg, 0.0_deg);
  auto north_pole_c = north_pole_dir.to_cartesian();
  auto equatorial_c = equatorial_dir.to_cartesian();

  std::cout << std::setprecision(6);
  std::cout << "dot(North Pole, Equatorial point)   = "
            << north_pole_c.dot(equatorial_c) << "  (must be  0)" << std::endl;

  // Anti-Polaris: opposite direction on the sky
  spherical::direction::EquatorialMeanJ2000 anti_polaris(
      qtty::Degree(polaris.ra().value() + 180.0),
      qtty::Degree(-polaris.dec().value()));
  auto anti_polaris_c = anti_polaris.to_cartesian();
  std::cout << "dot(Polaris, anti-Polaris)          = "
            << polaris_cart.dot(anti_polaris_c) << "  (must be -1)" << std::endl
            << std::endl;

  // =========================================================================
  // 4. Euclidean Distance Between Spherical Positions
  // =========================================================================
  std::cout << "4. EUCLIDEAN DISTANCE BETWEEN SPHERICAL POSITIONS" << std::endl;
  std::cout << "--------------------------------------------------"
            << std::endl;

  // Approximate heliocentric ecliptic positions (lon, lat, distance)
  spherical::position::EclipticMeanJ2000<qtty::AstronomicalUnit> earth(
      100.0_deg,                  // ecliptic longitude
      0.0_deg,                    // ecliptic latitude
      qtty::AstronomicalUnit(1.0) // 1 AU
  );
  spherical::position::EclipticMeanJ2000<qtty::AstronomicalUnit> mars(
      200.0_deg,                    // ecliptic longitude (~100° further)
      2.0_deg,                      // ecliptic latitude
      qtty::AstronomicalUnit(1.524) // ~1.524 AU
  );

  auto eu_dist = earth.distance_to(mars);
  auto ang_sep_pos = earth.angular_separation(mars);

  std::cout << std::setprecision(4);
  std::cout << "Earth (lon=100\u00b0, lat=0\u00b0, r=1.000 AU)" << std::endl;
  std::cout << "Mars  (lon=200\u00b0, lat=2\u00b0, r=1.524 AU)" << std::endl;
  std::cout << "  Euclidean 3D distance  = " << eu_dist << std::endl;
  std::cout << "  Angular separation     = " << ang_sep_pos << std::endl
            << std::endl;

  // =========================================================================
  // 5. Same Operations from Cartesian Positions
  // =========================================================================
  std::cout << "5. CARTESIAN POSITION OPERATIONS" << std::endl;
  std::cout << "---------------------------------" << std::endl;

  auto earth_cart = earth.to_cartesian();
  auto mars_cart = mars.to_cartesian();

  auto eu_dist_cart = earth_cart.distance_to(mars_cart);
  std::cout << "Earth cartesian (AU): (" << earth_cart.x() << ", "
            << earth_cart.y() << ", " << earth_cart.z() << ")" << std::endl;
  std::cout << "Mars  cartesian (AU): (" << mars_cart.x() << ", "
            << mars_cart.y() << ", " << mars_cart.z() << ")" << std::endl;
  std::cout << "  Euclidean distance = " << eu_dist_cart
            << "  (matches spherical)" << std::endl;

  // Vector difference gives displacement vector
  auto diff = mars_cart - earth_cart;
  std::cout << "  Mars \u2212 Earth vector: (" << diff.x() << ", " << diff.y()
            << ", " << diff.z() << ")" << std::endl;
  std::cout << "  |Mars \u2212 Earth|      = " << diff.magnitude() << std::endl
            << std::endl;

  // =========================================================================
  // 6. Ecliptic Directions — Same API, Different Frame
  // =========================================================================
  std::cout << "6. ECLIPTIC AND EQUATORIAL DIRECTIONS" << std::endl;
  std::cout << "--------------------------------------" << std::endl;

  // EclipticMeanJ2000 uses lon/lat convention: Direction(lon, lat)
  spherical::direction::EclipticMeanJ2000 vernal_equinox(0.0_deg, 0.0_deg);
  spherical::direction::EclipticMeanJ2000 summer_solstice(90.0_deg, 0.0_deg);
  spherical::direction::EclipticMeanJ2000 ecliptic_north(0.0_deg, 90.0_deg);

  std::cout << "Vernal Equinox \u2192 Summer Solstice ang. sep = "
            << vernal_equinox.angular_separation(summer_solstice)
            << "  (must be 90\u00b0)" << std::endl;
  std::cout << "Ecliptic North Pole \u2192 Vernal Equinox ang. sep = "
            << ecliptic_north.angular_separation(vernal_equinox)
            << "  (must be 90\u00b0)" << std::endl
            << std::endl;

  std::cout << "  Type safety note: spherical::direction::EclipticMeanJ2000 and"
            << std::endl;
  std::cout << "  spherical::direction::EquatorialMeanJ2000 are distinct types."
            << std::endl;
  std::cout << "  angular_separation only compiles within the same frame."
            << std::endl
            << std::endl;

  std::cout << "=== Example Complete ===" << std::endl;
  return 0;
}
