/**
 * @file altitude_events_example.cpp
 * @example altitude_events_example.cpp
 * @brief Altitude periods/crossings/culminations for multiple target types.
 */

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

#include <siderust/siderust.hpp>

namespace {

void print_periods(const std::vector<siderust::Period> &periods,
                   std::size_t limit) {
  const std::size_t n = std::min(periods.size(), limit);
  for (std::size_t i = 0; i < n; ++i) {
    const auto &p = periods[i];
    std::cout << "    " << (i + 1) << ") " << p.start().to_utc() << " -> "
              << p.end().to_utc() << " (" << std::fixed << std::setprecision(2)
              << p.duration<qtty::Hour>().value() << " h)\n";
  }
}

} // namespace

int main() {
  using namespace siderust;
  using namespace qtty::literals;

  const Geodetic obs = MAUNA_KEA;
  const MJD start = MJD::from_utc({2026, 7, 15, 0, 0, 0});
  const MJD end = start + qtty::Day(2.0);
  const Period window(start, end);

  SearchOptions opts;
  opts.with_tolerance(1e-9).with_scan_step(1.0 / 1440.0); // ~1 minute scan step

  std::cout << "=== altitude_events_example ===\n";
  std::cout << "Window: " << start.to_utc() << " -> " << end.to_utc() << "\n\n";

  auto sun_nights = sun::below_threshold(obs, window, -18.0_deg, opts);
  std::cout << "Sun below -18 deg (astronomical night): " << sun_nights.size()
            << " period(s)\n";
  print_periods(sun_nights, 3);

  auto sun_cross = sun::crossings(obs, window, 0.0_deg, opts);
  std::cout << "\nSun horizon crossings: " << sun_cross.size() << "\n";
  if (!sun_cross.empty()) {
    const auto &c = sun_cross.front();
    std::cout << "  First crossing: " << c.time.to_utc() << " (" << c.direction
              << ")\n";
  }

  auto moon_culm = moon::culminations(obs, window, opts);
  std::cout << "\nMoon culminations: " << moon_culm.size() << "\n";
  if (!moon_culm.empty()) {
    const auto &c = moon_culm.front();
    std::cout << "  First culmination: " << c.time.to_utc()
              << " kind=" << c.kind << " alt=" << c.altitude << std::endl;
  }

  auto vega_periods =
      star_altitude::above_threshold(VEGA, obs, window, 30.0_deg, opts);
  std::cout << "\nVega above 30 deg: " << vega_periods.size() << " period(s)\n";
  print_periods(vega_periods, 2);

  spherical::direction::ICRS target_dir(279.23473_deg, 38.78369_deg);
  auto dir_visible =
      icrs_altitude::above_threshold(target_dir, obs, window, 0.0_deg, opts);
  std::cout << "\nFixed ICRS direction above horizon: " << dir_visible.size()
            << " period(s)\n";

  ICRSTarget fixed_target{
      spherical::direction::ICRS{279.23473_deg, 38.78369_deg}};
  auto fixed_target_periods =
      fixed_target.above_threshold(obs, window, 45.0_deg, opts);
  std::cout << "ICRSTarget::above_threshold(45 deg): "
            << fixed_target_periods.size() << " period(s)\n";

  return 0;
}
