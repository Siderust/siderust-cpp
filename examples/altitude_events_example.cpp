/**
 * @file altitude_events_example.cpp
 * @brief Altitude windows, crossings, and culminations for Sun, Moon, and stars.
 *
 * Usage:
 *   cmake --build build-make --target altitude_events_example
 *   ./build-make/altitude_events_example
 */

#include <siderust/siderust.hpp>

#include <cstdio>
#include <vector>

using namespace siderust;
using namespace qtty::literals;

static const char* crossing_direction_name(CrossingDirection d) {
    return (d == CrossingDirection::Rising) ? "rising" : "setting";
}

static const char* culmination_kind_name(CulminationKind k) {
    return (k == CulminationKind::Max) ? "max" : "min";
}

static void print_utc(const UTC& utc) {
    std::printf("%04d-%02u-%02u %02u:%02u:%02u",
                utc.year, utc.month, utc.day,
                utc.hour, utc.minute, utc.second);
}

static void print_periods(const char* title, const std::vector<Period>& periods, std::size_t max_items = 4) {
    std::printf("%s: %zu period(s)\n", title, periods.size());
    const std::size_t n = (periods.size() < max_items) ? periods.size() : max_items;
    for (std::size_t i = 0; i < n; ++i) {
        const auto s = periods[i].start().to_utc();
        const auto e = periods[i].end().to_utc();
        std::printf("  %zu) ", i + 1);
        print_utc(s);
        std::printf(" -> ");
        print_utc(e);
        std::printf("  (%.2f h)\n", periods[i].duration_days() * 24.0);
    }
    if (periods.size() > n) {
        std::printf("  ... (%zu more)\n", periods.size() - n);
    }
}

static void print_crossings(const char* title, const std::vector<CrossingEvent>& events, std::size_t max_items = 6) {
    std::printf("%s: %zu event(s)\n", title, events.size());
    const std::size_t n = (events.size() < max_items) ? events.size() : max_items;
    for (std::size_t i = 0; i < n; ++i) {
        const auto t = events[i].time.to_utc();
        std::printf("  %zu) ", i + 1);
        print_utc(t);
        std::printf("  %s\n", crossing_direction_name(events[i].direction));
    }
    if (events.size() > n) {
        std::printf("  ... (%zu more)\n", events.size() - n);
    }
}

static void print_culminations(const char* title, const std::vector<CulminationEvent>& events, std::size_t max_items = 6) {
    std::printf("%s: %zu event(s)\n", title, events.size());
    const std::size_t n = (events.size() < max_items) ? events.size() : max_items;
    for (std::size_t i = 0; i < n; ++i) {
        const auto t = events[i].time.to_utc();
        std::printf("  %zu) ", i + 1);
        print_utc(t);
        std::printf("  alt=%.3f deg  kind=%s\n",
                    events[i].altitude.value(),
                    culmination_kind_name(events[i].kind));
    }
    if (events.size() > n) {
        std::printf("  ... (%zu more)\n", events.size() - n);
    }
}

int main() {
    std::printf("=== Altitude Events Example ===\n\n");

    const auto obs = MAUNA_KEA;
    const auto start = MJD::from_utc({2026, 7, 15, 0, 0, 0});
    const auto end = start + 2.0;
    const Period window(start, end);

    std::printf("Observer: Mauna Kea (lon=%.4f lat=%.4f h=%.0f m)\n",
                obs.lon.value(), obs.lat.value(), obs.height.value());
    std::printf("Window MJD: %.6f -> %.6f\n\n", start.value(), end.value());

    SearchOptions opts;
    opts.with_scan_step(1.0 / 144.0).with_tolerance(1e-10);

    // Sun examples.
    const auto sun_night = sun::below_threshold(obs, window, -18.0_deg, opts);
    const auto sun_cross = sun::crossings(obs, window, -0.833_deg, opts);
    const auto sun_culm = sun::culminations(obs, window, opts);
    print_periods("Sun below -18 deg (astronomical night)", sun_night);
    print_crossings("Sun crossings at -0.833 deg", sun_cross);
    print_culminations("Sun culminations", sun_culm);
    std::printf("\n");

    // Moon examples.
    const auto moon_above = moon::above_threshold(obs, window, 20.0_deg, opts);
    const auto moon_cross = moon::crossings(obs, window, 0.0_deg, opts);
    const auto moon_culm = moon::culminations(obs, window, opts);
    print_periods("Moon above +20 deg", moon_above);
    print_crossings("Moon horizon crossings", moon_cross);
    print_culminations("Moon culminations", moon_culm);
    std::printf("\n");

    // Star examples.
    const auto& vega = VEGA;
    const auto vega_above = star_altitude::above_threshold(vega, obs, window, 25.0_deg, opts);
    const auto vega_cross = star_altitude::crossings(vega, obs, window, 0.0_deg, opts);
    const auto vega_culm = star_altitude::culminations(vega, obs, window, opts);
    print_periods("VEGA above +25 deg", vega_above);
    print_crossings("VEGA horizon crossings", vega_cross);
    print_culminations("VEGA culminations", vega_culm);
    std::printf("\n");

    // Fixed ICRS direction examples.
    const spherical::direction::ICRS dir_icrs(279.23473_deg, 38.78369_deg);
    const auto dir_above = icrs_altitude::above_threshold(
        dir_icrs, obs, window, 30.0_deg, opts
    );
    const auto dir_below = icrs_altitude::below_threshold(
        dir_icrs, obs, window, 0.0_deg, opts
    );
    print_periods("Fixed ICRS direction above +30 deg", dir_above);
    print_periods("Fixed ICRS direction below horizon", dir_below);

    return 0;
}
