/**
 * @file 09_star_observability.cpp
 * @brief C++ port of siderust/examples/39_star_observability.rs
 *
 * Demonstrates using the altitude period API to plan optimal observing windows
 * for multiple catalog stars at a given observatory.
 *
 * Run with: cmake --build build --target star_observability_example
 */

#include <iomanip>
#include <iostream>
#include <vector>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

int main() {
    std::cout << "\u2554\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\n";
    std::cout << "\u2551         Star Observability Planner                    \u2551\n";
    std::cout << "\u255a\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\n\n";

    // Observatory: Greenwich
    const Geodetic obs = Geodetic(0.0, 51.4769, 0.0);
    std::cout << "Observatory: Greenwich Royal Observatory\n";
    std::cout << "  lat = 51.4769 N, lon = 0.0 E\n\n";

    // Tonight: one night starting at MJD 60000.5
    const MJD start(60000.5);
    const MJD end(60001.5);
    const Period night(start, end);
    std::cout << "Observation window: MJD " << start.value() << " to " << end.value() << "\n\n";

    // Find astronomical night (Sun below -18°)
    const auto dark_periods = sun::below_threshold(obs, night, qtty::Degree(-18.0));

    if (dark_periods.empty()) {
        std::cout << "No astronomical darkness available tonight!\n";
        return 0;
    }

    double total_dark_h = 0.0;
    for (const auto &p : dark_periods)
        total_dark_h += p.duration<qtty::Hour>().value();
    std::cout << "Astronomical night duration: " << std::fixed << std::setprecision(2)
              << total_dark_h << " h\n\n";

    // Target stars
    struct TargetInfo { const char *name; const Star *star; };
    const TargetInfo targets[] = {
        {"Sirius",     &SIRIUS},
        {"Vega",       &VEGA},
        {"Altair",     &ALTAIR},
        {"Betelgeuse", &BETELGEUSE},
        {"Rigel",      &RIGEL},
        {"Polaris",    &POLARIS},
    };

    const qtty::Degree min_alt(30.0); // minimum altitude for good observation

    std::cout << "\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\n";
    std::cout << "  Target Visibility (altitude > 30°, during astronomical night)\n";
    std::cout << "\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\n\n";

    for (const auto &t : targets) {
        const auto visible = star_altitude::above_threshold(*t.star, obs, night, min_alt);

        double total_h = 0.0;
        for (const auto &p : visible)
            total_h += p.duration<qtty::Hour>().value();

        std::cout << std::left << std::setw(12) << t.name
                  << "  periods=" << std::setw(3) << visible.size()
                  << "  total=" << std::setprecision(2) << std::fixed
                  << total_h << " h\n";

        if (!visible.empty()) {
            const auto &best = visible.front();
            std::cout << "             first period: MJD "
                      << std::setprecision(4) << best.start().value()
                      << " → " << best.end().value()
                      << " (" << std::setprecision(2) << best.duration<qtty::Hour>().value()
                      << " h)\n";
        }
    }

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
