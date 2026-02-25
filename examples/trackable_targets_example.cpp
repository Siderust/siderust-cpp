/**
 * @file trackable_targets_example.cpp
 * @example trackable_targets_example.cpp
 * @brief Using DirectionTarget<C>, StarTarget, BodyTarget through the Target
 * polymorphic interface.
 *
 * Demonstrates the strongly-typed DirectionTarget template with multiple frames:
 *   - ICRSTarget          — fixed direction in ICRS equatorial coordinates
 *   - EclipticMeanJ2000Target — fixed direction in mean ecliptic J2000
 *
 * Non-ICRS targets are silently converted to ICRS at construction time for
 * the Rust FFI layer; the original typed direction is retained in C++.
 */

#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <siderust/siderust.hpp>

int main() {
  using namespace siderust;

  const Geodetic site = geodetic(-17.8890, 28.7610, 2396.0);
  const MJD now = MJD::from_utc({2026, 7, 15, 22, 0, 0});
  const Period window(now, now + qtty::Day(1.0));

  std::cout << "=== trackable_targets_example ===\n";
  std::cout << "Epoch UTC: " << now.to_utc() << "\n\n";

  // Strongly-typed ICRS target — ra() / dec() return qtty::Degree.
  ICRSTarget fixed_vega_like{spherical::direction::ICRS{
      qtty::Degree(279.23473), qtty::Degree(38.78369)}};
  std::cout << "ICRSTarget metadata\n";
  std::cout << "  name=" << fixed_vega_like.name() << "\n";
  std::cout << "  RA/Dec=" << fixed_vega_like.direction()
            << "  epoch=" << fixed_vega_like.epoch() << " JD\n\n";

  // Ecliptic target (Vega in EclipticMeanJ2000, lon≈279.6°, lat≈+61.8°).
  // Automatically converted to ICRS by the constructor.
  EclipticMeanJ2000Target ecliptic_vega{spherical::direction::EclipticMeanJ2000{
      qtty::Degree(279.6), qtty::Degree(61.8)}};
  auto alt_ecliptic = ecliptic_vega.altitude_at(site, now);
  std::cout << "EclipticMeanJ2000Target (Vega approx)\n";
  std::cout << "  name=" << ecliptic_vega.name() << "\n";
  std::cout << "  ecl lon/lat=" << ecliptic_vega.direction() << "\n";
  std::cout << "  ICRS ra/dec=" << ecliptic_vega.icrs_direction()
            << " (converted)\n";
  std::cout << "  alt=" << alt_ecliptic << "\n\n";

  // Polymorphic catalog: all targets share the Target base.
  // DirectionTarget accepts an optional label at construction.
  std::vector<std::unique_ptr<Target>> catalog;
  catalog.push_back(std::make_unique<BodyTarget>(Body::Sun));
  catalog.push_back(std::make_unique<BodyTarget>(Body::Mars));
  catalog.push_back(std::make_unique<StarTarget>(VEGA));
  catalog.push_back(std::make_unique<ICRSTarget>(
      spherical::direction::ICRS{qtty::Degree(279.23473), qtty::Degree(38.78369)},
      JulianDate::J2000(), "Vega (ICRS coord)"));

  for (const auto &t : catalog) {
    auto alt = t->altitude_at(site, now);
    auto az  = t->azimuth_at(site, now);

    std::cout << std::left << std::setw(22) << t->name() << std::right
              << " alt=" << std::setw(9) << alt << " az=" << az << std::endl;

    auto crossings = t->crossings(site, window, qtty::Degree(0.0));
    if (!crossings.empty()) {
      const auto &first = crossings.front();
      std::cout << "  first horizon crossing: " << first.time.to_utc() << " ("
                << first.direction << ")\n";
    }

    auto az_cross = t->azimuth_crossings(site, window, qtty::Degree(180.0));
    if (!az_cross.empty()) {
      std::cout << "  first az=180 crossing: " << az_cross.front().time.to_utc()
                << "\n";
    }
  }

  return 0;
}
