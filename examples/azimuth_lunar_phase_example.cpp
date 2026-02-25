/**
 * @file azimuth_lunar_phase_example.cpp
 * @example azimuth_lunar_phase_example.cpp
 * @brief Azimuth event search plus lunar phase geometry/events.
 */

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

#include <siderust/siderust.hpp>

namespace {} // namespace

int main() {
  using namespace siderust;
  using namespace qtty::literals;

  const Geodetic site = MAUNA_KEA;
  const MJD start = MJD::from_utc({2026, 7, 15, 0, 0, 0});
  const MJD end = start + qtty::Day(3.0);
  const Period window(start, end);

  std::cout << "=== azimuth_lunar_phase_example ===\n";
  std::cout << "Window UTC: " << start.to_utc() << " -> " << end.to_utc()
            << "\n\n";

  const MJD now = MJD::from_utc({2026, 7, 15, 12, 0, 0});
  std::cout << "Instant azimuth\n";
  std::cout << "  Sun  : " << sun::azimuth_at(site, now) << std::endl;
  std::cout << "  Moon : " << moon::azimuth_at(site, now) << std::endl;
  std::cout << "  Vega : " << star_altitude::azimuth_at(VEGA, site, now)
            << std::endl;

  auto sun_cross = sun::azimuth_crossings(site, window, 180.0_deg);
  auto sun_ext = sun::azimuth_extrema(site, window);
  auto moon_west = moon::in_azimuth_range(site, window, 240.0_deg, 300.0_deg);

  std::cout << "Azimuth events\n";
  std::cout << "  Sun crossings at 180 deg: " << sun_cross.size() << "\n";
  std::cout << "  Sun azimuth extrema: " << sun_ext.size() << "\n";
  if (!sun_ext.empty()) {
    const auto &e = sun_ext.front();
    std::cout << "    first extremum " << e.kind << " at " << e.time.to_utc()
              << " az=" << e.azimuth << std::endl;
  }
  std::cout << "  Moon in [240,300] deg azimuth: " << moon_west.size()
            << " period(s)\n\n";

  const JulianDate jd_now = now.to_jd();
  auto geo_phase = moon::phase_geocentric(jd_now);
  auto topo_phase = moon::phase_topocentric(jd_now, site);
  auto topo_label = moon::phase_label(topo_phase);

  auto phase_events =
      moon::find_phase_events(Period(start, start + qtty::Day(30.0)));
  auto half_lit = moon::illumination_range(window, 0.45, 0.55);

  std::cout << "Lunar phase\n";
  std::cout << std::fixed << std::setprecision(3)
            << "  Geocentric illuminated fraction: "
            << geo_phase.illuminated_fraction << "\n"
            << "  Topocentric illuminated fraction: "
            << topo_phase.illuminated_fraction << " (" << topo_label << ")\n";

  std::cout << "  Principal phase events in next 30 days: "
            << phase_events.size() << "\n";
  const std::size_t n = std::min<std::size_t>(phase_events.size(), 4);
  for (std::size_t i = 0; i < n; ++i) {
    const auto &ev = phase_events[i];
    std::cout << "    " << ev.time.to_utc() << " -> " << ev.kind << "\n";
  }

  std::cout << "  Near-half illumination periods (k in [0.45, 0.55]): "
            << half_lit.size() << "\n";

  return 0;
}
