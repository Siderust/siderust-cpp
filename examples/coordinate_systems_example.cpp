/**
 * @file coordinate_systems_example.cpp
 * @example coordinate_systems_example.cpp
 * @brief Compile-time frame tags and transform capabilities walkthrough.
 */

#include <iomanip>
#include <iostream>
#include <type_traits>

#include <siderust/siderust.hpp>

int main() {
  using namespace siderust;
  using namespace siderust::frames;

  std::cout << "=== coordinate_systems_example ===\n";

  static_assert(has_frame_transform_v<ICRS, EclipticMeanJ2000>);
  static_assert(has_frame_transform_v<EclipticMeanJ2000, EquatorialTrueOfDate>);
  static_assert(has_horizontal_transform_v<ICRS>);

  const Geodetic observer = ROQUE_DE_LOS_MUCHACHOS;
  const auto ecef = observer.to_cartesian<qtty::Kilometer>();

  const JulianDate jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});

  spherical::Direction<ICRS> src(qtty::Degree(279.23473),
                                 qtty::Degree(38.78369));
  const auto ecl = src.to_frame<EclipticMeanJ2000>(jd);
  const auto mod = src.to_frame<EquatorialMeanOfDate>(jd);
  const auto tod = mod.to_frame<EquatorialTrueOfDate>(jd);
  const auto horiz = src.to_horizontal(jd, observer);

  std::cout << std::fixed << std::setprecision(6);
  std::cout << "Observer: " << observer << std::endl;
  std::cout << "Observer in ECEF: " << ecef << std::endl;

  std::cout << "Frame transforms for Vega-like direction\n";
  std::cout << "  ICRS RA/Dec                : " << src << "\n";
  std::cout << "  EclipticMeanJ2000 lon/lat  : " << ecl << "\n";
  std::cout << "  EquatorialMeanOfDate RA/Dec: " << mod << "\n";
  std::cout << "  EquatorialTrueOfDate RA/Dec: " << tod << "\n";
  std::cout << "  Horizontal az/alt          : " << horiz << "\n";

  return 0;
}
