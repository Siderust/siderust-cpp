#include <siderust/siderust.hpp>

#include <cmath>
#include <iostream>
#include <iomanip>

using namespace siderust;

// Approximate Sun–Earth L2 offset: 1.5e6 km beyond Earth along the Sun–Earth
// line. Convert to AU so we can stay unit-safe with qtty.
constexpr double L2_OFFSET_KM = 1'500'000.0;
constexpr double KM_PER_AU = 149'597'870.7;
constexpr double L2_OFFSET_AU = L2_OFFSET_KM / KM_PER_AU;

cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>
compute_l2_heliocentric(const JulianDate &jd) {
  const auto earth = ephemeris::earth_heliocentric(jd);

  // Unit vector from Sun → Earth (heliocentric frame).
  const double ex = earth.x().value();
  const double ey = earth.y().value();
  const double ez = earth.z().value();
  const double norm = std::sqrt(ex * ex + ey * ey + ez * ez);
  const double ux = ex / norm;
  const double uy = ey / norm;
  const double uz = ez / norm;

  // Move ~0.01 AU further from the Sun along that direction.
  const qtty::AstronomicalUnit offset(L2_OFFSET_AU);
  return {earth.x() + offset * ux, earth.y() + offset * uy,
          earth.z() + offset * uz};
}

cartesian::Position<centers::Bodycentric, frames::EclipticMeanJ2000,
                    qtty::AstronomicalUnit>
mars_relative_to_l2(const JulianDate &jd) {
  const auto mars = ephemeris::mars_heliocentric(jd);
  const auto l2 = compute_l2_heliocentric(jd);

  // Bodycentric position = target - observer.
  return {mars.x() - l2.x(), mars.y() - l2.y(), mars.z() - l2.z()};
}

int main() {
  std::cout << "╔══════════════════════════════════════════╗\n"
            << "║  Mars as Seen from a JWST-like L2 Orbit   ║\n"
            << "╚══════════════════════════════════════════╝\n\n";

  const JulianDate obs_epoch(2460000.0); // ~2023-06-30

  std::cout << "Observation epoch (JD): " << std::fixed << std::setprecision(1)
            << obs_epoch.value() << "\n\n";

  const auto mars_helio = ephemeris::mars_heliocentric(obs_epoch);
  const auto l2_helio = compute_l2_heliocentric(obs_epoch);
  const auto mars_from_l2 = mars_relative_to_l2(obs_epoch);

  std::cout << "Mars heliocentric (EclipticMeanJ2000):\n  " << mars_helio
            << "\n\n";

  std::cout << "L2 heliocentric (Earth + 1.5e6 km radial):\n  " << l2_helio
            << "\n\n";

  std::cout << "Mars relative to L2 (bodycentric):\n  " << mars_from_l2
            << "\n";

  return 0;
}
