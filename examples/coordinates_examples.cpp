/**
 * @file coordinates_examples.cpp
 * @example coordinates_examples.cpp
 * @brief Focused typed-coordinate construction and conversion examples.
 */

#include <iomanip>
#include <iostream>
#include <type_traits>

#include <siderust/siderust.hpp>

int main() {
  using namespace siderust;
  using namespace qtty::literals;

  std::cout << "=== coordinates_examples ===\n";

  const Geodetic site(-17.8890_deg, 28.7610_deg, 2396.0_m);
  const auto ecef_m = site.to_cartesian();
  const auto ecef_km = site.to_cartesian<qtty::Kilometer>();

  static_assert(std::is_same_v<std::remove_cv_t<decltype(ecef_m)>,
                               cartesian::position::ECEF<qtty::Meter>>);

  std::cout << "Geodetic -> ECEF \n " << site << "\n"
            << ecef_m << "\n"
            << "(" << ecef_km << ")\n"
            << std::endl;

  const JulianDate jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});

  spherical::direction::ICRS vega_icrs(279.23473_deg, 38.78369_deg);
  auto vega_ecl = vega_icrs.to_frame<frames::EclipticMeanJ2000>(jd);
  auto vega_true = vega_icrs.to_frame<frames::EquatorialTrueOfDate>(jd);
  auto vega_horiz = vega_icrs.to_horizontal(jd, site);

  std::cout << "Direction transforms\n";
  std::cout << "  ICRS RA/Dec: " << vega_icrs << std::endl;
  std::cout << "  Ecliptic lon/lat: " << vega_ecl << std::endl;
  std::cout << "  True-of-date RA/Dec: " << vega_true << std::endl;
  std::cout << "  Horizontal az/alt: " << vega_horiz << std::endl;

  spherical::position::ICRS<qtty::AstronomicalUnit> synthetic_star(
      210.0_deg, -12.0_deg, 4.2_au);

  cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit> earth =
      ephemeris::earth_heliocentric(jd);

  std::cout << "Typed positions\n";
  std::cout << "  Synthetic star distance: " << synthetic_star.distance()
            << std::endl;
  std::cout << "  Earth heliocentric x: " << earth.x() << std::endl;

  return 0;
}
