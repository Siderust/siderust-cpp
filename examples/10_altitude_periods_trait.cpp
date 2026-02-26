/**
 * @file 10_altitude_periods_trait.cpp
 * @brief C++ port of siderust/examples/24_altitude_periods_trait.rs
 *
 * Demonstrates the unified altitude period API (implemented via the
 * `Target` base class and free-function namespaces) for finding time
 * intervals when celestial bodies are within specific altitude ranges.
 *
 * In Rust this is the `AltitudePeriodsProvider` trait. In C++ the same
 * functionality is available via:
 *   - `sun::above_threshold / below_threshold`
 *   - `moon::above_threshold / below_threshold`
 *   - `star_altitude::above_threshold / below_threshold`
 *   - Polymorphic via the `Target` / `BodyTarget` / `StarTarget` classes
 *
 * Run with: cmake --build build --target altitude_periods_trait_example
 */

#include <iomanip>
#include <iostream>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

int main() {
    std::cout << "=== Altitude Periods API Examples ===\n\n";

    const Geodetic observer = ROQUE_DE_LOS_MUCHACHOS;
    std::cout << "Observer: Roque de los Muchachos Observatory\n\n";

    // Time window: one week starting from MJD 60000
    const MJD start(60000.0);
    const MJD end(60007.0);
    const Period window(start, end);
    std::cout << "Time window: MJD " << std::fixed << std::setprecision(1)
              << start.value() << " → " << end.value() << " (7 days)\n\n";

    // -------------------------------------------------------------------------
    // Example 1: Astronomical nights (Sun below -18°)
    // -------------------------------------------------------------------------
    std::cout << "--- Example 1: Astronomical Nights ---\n";
    const auto astro_nights = sun::below_threshold(observer, window, qtty::Degree(-18.0));

    std::cout << "Found " << astro_nights.size() << " astronomical night period(s):\n";
    for (size_t i = 0; i < std::min(astro_nights.size(), size_t(3)); ++i) {
        const auto &p = astro_nights[i];
        std::cout << "  Night " << i + 1 << ": "
                  << std::setprecision(3) << p.duration<qtty::Hour>().value() << " h\n";
    }
    std::cout << "\n";

    // -------------------------------------------------------------------------
    // Example 2: Sirius above 30°
    // -------------------------------------------------------------------------
    std::cout << "--- Example 2: Sirius High Above Horizon ---\n";
    const auto sirius_high = star_altitude::above_threshold(SIRIUS, observer, window,
                                                             qtty::Degree(30.0));
    std::cout << "Sirius above 30° altitude:\n";
    std::cout << "  Found " << sirius_high.size() << " period(s)\n";
    if (!sirius_high.empty()) {
        std::cout << "  First period: " << std::setprecision(3)
                  << sirius_high.front().duration<qtty::Hour>().value() << " h\n";
    }
    std::cout << "\n";

    // -------------------------------------------------------------------------
    // Example 3: Custom ICRS direction (Betelgeuse) — above horizon
    // -------------------------------------------------------------------------
    std::cout << "--- Example 3: Custom ICRS Direction (Betelgeuse) ---\n";
    // Betelgeuse: RA ≈ 88.79°, Dec ≈ +7.41°
    const spherical::direction::ICRS betelgeuse_dir(qtty::Degree(88.79),
                                                     qtty::Degree(7.41));
    // Use StarTarget with a custom (non-catalog) star to demonstrate the API
    // (Alternatively use the catalog BETELGEUSE star handle)
    const auto btel_visible = star_altitude::above_threshold(BETELGEUSE, observer, window,
                                                              qtty::Degree(0.0));
    double total_btel_h = 0.0;
    for (const auto &p : btel_visible)
        total_btel_h += p.duration<qtty::Hour>().value();
    std::cout << "Betelgeuse above horizon:\n";
    std::cout << "  Found " << btel_visible.size() << " period(s) in 7 days\n";
    std::cout << "  Total visible time: " << std::setprecision(2) << total_btel_h << " h\n\n";

    // -------------------------------------------------------------------------
    // Example 4: Moon altitude range query (0° to 20°)
    // -------------------------------------------------------------------------
    std::cout << "--- Example 4: Low Moon Periods (0° to 20°) ---\n";
    const auto moon_low  = moon::above_threshold(observer, window, qtty::Degree(0.0));
    const auto moon_high = moon::above_threshold(observer, window, qtty::Degree(20.0));

    // Low moon = above 0° minus above 20° (approximate count; periods may differ)
    std::cout << "Moon between 0° and 20° altitude (approx):\n";
    std::cout << "  Moon >  0°: " << moon_low.size() << " period(s)\n";
    std::cout << "  Moon > 20°: " << moon_high.size() << " period(s)\n\n";

    // -------------------------------------------------------------------------
    // Example 5: Circumpolar star check (Polaris)
    // -------------------------------------------------------------------------
    std::cout << "--- Example 5: Circumpolar Star (Polaris) ---\n";
    const auto polaris_up = star_altitude::above_threshold(POLARIS, observer, window,
                                                            qtty::Degree(0.0));

    double total_polaris_h = 0.0;
    for (const auto &p : polaris_up)
        total_polaris_h += p.duration<qtty::Hour>().value();

    std::cout << "Polaris above horizon:\n";
    if (polaris_up.size() == 1 && std::abs(total_polaris_h - 7.0 * 24.0) < 2.4) {
        std::cout << "  Circumpolar (continuously visible for entire week)\n";
    } else {
        std::cout << "  Found " << polaris_up.size() << " period(s), "
                  << "total " << std::setprecision(2) << total_polaris_h << " h\n";
    }
    std::cout << "\n";

    // -------------------------------------------------------------------------
    // Example 6: Polymorphic dispatch via Target base class
    // -------------------------------------------------------------------------
    std::cout << "--- Example 6: Polymorphic Target Dispatch ---\n";
    std::vector<std::unique_ptr<Target>> targets;
    targets.push_back(std::make_unique<BodyTarget>(Body::Sun));
    targets.push_back(std::make_unique<BodyTarget>(Body::Moon));
    targets.push_back(std::make_unique<StarTarget>(VEGA));
    targets.push_back(std::make_unique<StarTarget>(SIRIUS));

    for (const auto &t : targets) {
        const auto periods = t->above_threshold(observer, window, qtty::Degree(0.0));
        double total_h = 0.0;
        for (const auto &p : periods)
            total_h += p.duration<qtty::Hour>().value();
        std::cout << "  " << std::left << std::setw(8) << t->name()
                  << "  " << std::setw(3) << periods.size()
                  << " periods  total = " << std::setprecision(2) << total_h << " h\n";
    }

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
