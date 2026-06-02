// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

#include <siderust/siderust.hpp>

#include <cmath>
#include <iostream>

using namespace siderust;
using namespace siderust::frames;
using namespace siderust::centers;
using AU = qtty::AstronomicalUnit;

int main() {
  std::cout << "=== Coordinate Transformations Example ===\n\n";

  const auto jd = JulianDate::J2000();
  std::cout << "Reference epoch: " << jd << "\n\n";

  cartesian::Position<Heliocentric, EclipticMeanJ2000, AU> pos_ecliptic(1.0, 0.0, 0.0);
  std::cout << "Heliocentric ecliptic:     " << pos_ecliptic << '\n';
  std::cout << "  -> equatorial:            " << pos_ecliptic.to_frame<EquatorialMeanJ2000>(jd)
            << '\n';
  std::cout << "  -> ICRS:                  "
            << pos_ecliptic.to_frame<EquatorialMeanJ2000>(jd).to_frame<ICRS>(jd) << "\n\n";

  auto earth_helio = ephemeris::earth_heliocentric(jd);
  std::cout << "Earth (heliocentric):      " << earth_helio << '\n';
  std::cout << "  -> geocentric (origin):  " << earth_helio.to_center<Geocentric>(jd) << '\n';

  auto mars_helio = ephemeris::mars_heliocentric(jd);
  std::cout << "Mars (heliocentric):       " << mars_helio << '\n';
  std::cout << "  -> geocentric:            " << mars_helio.to_center<Geocentric>(jd) << "\n\n";

  auto mars_geo_equ = mars_helio.transform<Geocentric, EquatorialMeanJ2000>(jd);
  std::cout << "Mars geocentric equatorial: " << mars_geo_equ << '\n';
  std::cout << "  (via .transform<Geocentric, EquatorialMeanJ2000>)\n\n";

  auto earth_bary = ephemeris::earth_barycentric(jd);
  std::cout << "Earth barycentric:         " << earth_bary << '\n';
  std::cout << "  -> geocentric:            " << earth_bary.to_center<Geocentric>(jd) << '\n';
  auto mars_geo_from_bary = ephemeris::mars_barycentric(jd).to_center<Geocentric>(jd);
  std::cout << "Mars geocentric (from bary): " << mars_geo_from_bary << "\n\n";

  cartesian::position::ICRS<AU> star_icrs(100.0, 50.0, 1000.0);
  std::cout << "Catalog star (barycentric ICRS): " << star_icrs << '\n';
  std::cout << "  -> geocentric ICRS (GCRS):       " << star_icrs.to_center<Geocentric>(jd)
            << "\n\n";

  auto recovered = mars_helio.transform<Geocentric, EquatorialMeanJ2000>(jd)
                       .transform<Heliocentric, EclipticMeanJ2000>(jd);
  std::cout << "Mars round-trip:           " << recovered << '\n';
  std::cout << "  |Δx| = " << std::abs(mars_helio.x().value() - recovered.x().value()) << " AU\n";
  return 0;
}
