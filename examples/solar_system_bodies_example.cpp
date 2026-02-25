/**
 * @file solar_system_bodies_example.cpp
 * @example solar_system_bodies_example.cpp
 * @brief Solar-system body catalog, ephemeris, and body-dispatch examples.
 */

#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

#include <siderust/siderust.hpp>

namespace {

const char *az_kind_name(siderust::AzimuthExtremumKind kind) {
  using siderust::AzimuthExtremumKind;
  switch (kind) {
  case AzimuthExtremumKind::Max:
    return "max";
  case AzimuthExtremumKind::Min:
    return "min";
  }
  return "unknown";
}

} // namespace

int main() {
  using namespace siderust;

  const Geodetic site = MAUNA_KEA;
  const JulianDate jd = JulianDate::from_utc({2026, 7, 15, 0, 0, 0});
  const MJD now = MJD::from_jd(jd);
  const Period window(now, now + qtty::Day(2.0));

  std::cout << "=== solar_system_bodies_example ===\n";
  std::cout << "Epoch UTC: " << jd.to_utc() << "\n\n";

  std::cout << "Planet catalog constants\n";
  std::cout << "  Mercury  a=" << MERCURY.orbit.semi_major_axis_au << " AU"
            << " radius=" << MERCURY.radius_km << " km\n";
  std::cout << "  Earth    a=" << EARTH.orbit.semi_major_axis_au << " AU"
            << " radius=" << EARTH.radius_km << " km\n";
  std::cout << "  Jupiter  a=" << JUPITER.orbit.semi_major_axis_au << " AU"
            << " radius=" << JUPITER.radius_km << " km\n\n";

  auto earth = ephemeris::earth_heliocentric(jd);
  auto moon_pos = ephemeris::moon_geocentric(jd);
  double moon_dist_km = std::sqrt(moon_pos.x().value() * moon_pos.x().value() +
                                  moon_pos.y().value() * moon_pos.y().value() +
                                  moon_pos.z().value() * moon_pos.z().value());

  std::cout << "Ephemeris\n";
  std::cout << std::fixed << std::setprecision(6)
            << "  Earth heliocentric x=" << earth.x().value()
            << " AU y=" << earth.y().value() << " AU\n";
  std::cout << std::setprecision(2)
            << "  Moon geocentric distance=" << moon_dist_km << " km\n\n";

  std::vector<Body> tracked = {Body::Sun, Body::Moon, Body::Mars,
                               Body::Jupiter};

  std::cout << "Body dispatch API at observer\n";
  for (Body b : tracked) {
    auto alt = body::altitude_at(b, site, now).to<qtty::Degree>();
    auto az = body::azimuth_at(b, site, now).to<qtty::Degree>();
    std::cout << "  body=" << static_cast<int>(b) << " alt=" << alt
              << " az=" << az << std::endl;
  }

  auto moon_extrema = body::azimuth_extrema(Body::Moon, site, window);
  if (!moon_extrema.empty()) {
    const auto &e = moon_extrema.front();
    std::cout << "\nMoon azimuth extrema\n";
    std::cout << "  first " << az_kind_name(e.kind) << " at " << e.time.to_utc()
              << " az=" << e.azimuth << std::endl;
  }

  return 0;
}
