/**
 * @file 21_trackable_demo.cpp
 * @brief C++ port of siderust/examples/42_trackable_demo.rs
 *
 * Demonstrates the unified `Target` polymorphism in siderust-cpp.  Any mix
 * of solar-system bodies, catalog stars, and fixed sky directions can be held
 * in a `std::vector<std::unique_ptr<Target>>` and queried uniformly via the
 * `Target` virtual interface.
 *
 * Rust counterpart uses the `Trackable` trait; C++ uses virtual dispatch on
 * `siderust::Target`.
 *
 * Run with: cmake --build build --target trackable_demo_example
 */

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

// ============================================================================
// Helpers
// ============================================================================

static void print_rising_setting(const Target &target, const Geodetic &obs,
                                  const Period &window) {
    const auto events = target.crossings(obs, window, qtty::Degree(0.0));
    for (const auto &ev : events) {
        const char *kind = (ev.direction == CrossingDirection::Rising) ? "Rise" : "Set";
        std::cout << "      " << kind << " at MJD "
                  << std::fixed << std::setprecision(4) << ev.time.value() << "\n";
    }
}

static void print_culminations(const Target &target, const Geodetic &obs,
                                const Period &window) {
    const auto culms = target.culminations(obs, window);
    for (const auto &cu : culms) {
        std::cout << "      Culmination at MJD "
                  << std::fixed << std::setprecision(4) << cu.time.value()
                  << "  alt=" << std::setprecision(2) << cu.altitude.value() << "°\n";
    }
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "=== Trackable Target Polymorphism Demo ===\n\n";

    // -------------------------------------------------------------------------
    // Build a heterogeneous list of targets
    // -------------------------------------------------------------------------
    std::vector<std::unique_ptr<Target>> targets;

    // Solar-system bodies
    targets.push_back(std::make_unique<BodyTarget>(Body::Sun));
    targets.push_back(std::make_unique<BodyTarget>(Body::Moon));
    targets.push_back(std::make_unique<BodyTarget>(Body::Mars));
    targets.push_back(std::make_unique<BodyTarget>(Body::Jupiter));

    // Catalog stars
    targets.push_back(std::make_unique<StarTarget>(SIRIUS));
    targets.push_back(std::make_unique<StarTarget>(VEGA));
    targets.push_back(std::make_unique<StarTarget>(POLARIS));
    targets.push_back(std::make_unique<StarTarget>(BETELGEUSE));
    targets.push_back(std::make_unique<StarTarget>(RIGEL));
    targets.push_back(std::make_unique<StarTarget>(CANOPUS));

    std::cout << "Target list (" << targets.size() << " objects):\n";
    for (const auto &t : targets)
        std::cout << "  • " << t->name() << "\n";
    std::cout << "\n";

    // -------------------------------------------------------------------------
    // Common observation parameters
    // -------------------------------------------------------------------------
    const Geodetic obs = EL_PARANAL;
    std::cout << "Observatory: ESO Paranal / VLT\n\n";

    const Period night(MJD(60000.5), MJD(60001.5));
    std::cout << "Observation night: MJD " << std::fixed << std::setprecision(2)
              << night.start().value() << " → " << night.end().value() << "\n\n";

    // -------------------------------------------------------------------------
    // Generic altitude survey via Target interface
    // -------------------------------------------------------------------------
    std::cout << "=== Altitude at Transit Middle (MJD 60001.0) ===\n";
    const MJD midpoint(60001.0);

    std::cout << std::left << std::setw(14) << "Target"
              << std::right << std::setw(12) << "Altitude (°)"
              << "   \n";
    std::cout << std::string(28, '-') << "\n";

    for (const auto &t : targets) {
        const double alt = t->altitude_at(obs, midpoint).value();
        std::cout << std::left << std::setw(14) << t->name()
                  << std::right << std::setw(12) << std::fixed << std::setprecision(2)
                  << alt << "°\n";
    }
    std::cout << "\n";

    // -------------------------------------------------------------------------
    // Above-threshold summary (30°)
    // -------------------------------------------------------------------------
    const qtty::Degree threshold(30.0);
    std::cout << "=== Time Above " << std::setprecision(0) << threshold.value()
              << "° During the Night ===\n\n";

    for (const auto &t : targets) {
        const auto periods = t->above_threshold(obs, night, threshold);
        double total_h = 0.0;
        for (const auto &p : periods)
            total_h += p.duration<qtty::Hour>().value();

        std::cout << std::left << std::setw(14) << t->name()
                  << "  " << periods.size() << " period(s)  "
                  << std::setprecision(2) << total_h << " h total\n";
    }
    std::cout << "\n";

    // -------------------------------------------------------------------------
    // Rising / setting / culmination events
    // -------------------------------------------------------------------------
    std::cout << "=== Rising, Setting & Culmination Events ===\n\n";

    for (size_t i = 0; i < std::min(targets.size(), size_t(4)); ++i) {
        const auto &t = targets[i];
        std::cout << "  " << t->name() << ":\n";
        print_rising_setting(*t, obs, night);
        print_culminations(*t, obs, night);
    }
    std::cout << "\n";

    // -------------------------------------------------------------------------
    // Sort targets by tonight's availability (most hours first)
    // -------------------------------------------------------------------------
    struct Availability {
        std::string name;
        double hours;
    };

    std::vector<Availability> avail;
    for (const auto &t : targets) {
        const auto ps = t->above_threshold(obs, night, qtty::Degree(20.0));
        double h = 0.0;
        for (const auto &p : ps)
            h += p.duration<qtty::Hour>().value();
        avail.push_back({t->name(), h});
    }

    std::sort(avail.begin(), avail.end(),
              [](const Availability &a, const Availability &b) {
                  return a.hours > b.hours;
              });

    std::cout << "=== Tonight's Observing Priority (sorted by hours >20°) ===\n\n";
    for (size_t i = 0; i < avail.size(); ++i) {
        std::cout << "  " << std::setw(2) << i + 1 << ". "
                  << std::left << std::setw(14) << avail[i].name
                  << std::right << std::setprecision(2) << avail[i].hours << " h\n";
    }

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
