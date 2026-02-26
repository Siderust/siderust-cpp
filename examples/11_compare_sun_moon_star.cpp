/**
 * @file 11_compare_sun_moon_star.cpp
 * @brief C++ port of siderust/examples/29_compare_sun_moon_star.rs
 *
 * Demonstrates generic altitude analysis for the Sun, Moon, and a star using
 * a single helper function that works with the polymorphic `Target` base class.
 *
 * Run with: cmake --build build --target compare_sun_moon_star_example
 */

#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

struct Summary {
    std::string name;
    size_t       period_count;
    double       total_hours;
    double       max_period_hours;
};

/// Generic analysis for any `Target` subclass.
static Summary analyze_body(const Target &body, const Geodetic &obs, const Period &window,
                             qtty::Degree threshold) {
    const auto periods = body.above_threshold(obs, window, threshold);

    double total        = 0.0;
    double max_duration = 0.0;

    for (const auto &p : periods) {
        const double h = p.duration<qtty::Hour>().value();
        total        += h;
        if (h > max_duration)
            max_duration = h;
    }

    return {body.name(), periods.size(), total, max_duration};
}

static void print_summary(const Summary &s) {
    std::cout << std::left << std::setw(12) << s.name
              << "  periods=" << std::setw(4) << s.period_count
              << "  total=" << std::setw(8) << std::setprecision(2) << std::fixed
              << s.total_hours << " h"
              << "  longest=" << std::setprecision(2) << s.max_period_hours << " h\n";
}

int main() {
    std::cout << "=== Comparing Sun, Moon, and Star Altitude Periods ===\n\n";

    const Geodetic observer = EL_PARANAL;
    std::cout << "Observatory: ESO Paranal / VLT\n\n";

    // 14-day window around a new moon
    const Period window(MJD(60000.0), MJD(60014.0));
    std::cout << "Time window: 14 days starting MJD 60000\n\n";

    // Build target list using polymorphic base class
    std::vector<std::unique_ptr<Target>> bodies;
    bodies.push_back(std::make_unique<BodyTarget>(Body::Sun));
    bodies.push_back(std::make_unique<BodyTarget>(Body::Moon));
    bodies.push_back(std::make_unique<StarTarget>(SIRIUS));
    bodies.push_back(std::make_unique<StarTarget>(VEGA));
    bodies.push_back(std::make_unique<StarTarget>(CANOPUS));

    const qtty::Degree threshold_deg(20.0);
    std::cout << "Altitude threshold: " << threshold_deg.value() << "°\n\n";

    std::cout << std::string(72, '=') << "\n";
    std::cout << "  Target       Periods    Total Time    Longest Period\n";
    std::cout << std::string(72, '-') << "\n";

    for (const auto &b : bodies) {
        const auto s = analyze_body(*b, observer, window, threshold_deg);
        print_summary(s);
    }

    std::cout << std::string(72, '=') << "\n\n";

    // -------------------------------------------------------------------------
    // Focused comparison: time simultaneously above 20° (Sun excluded)
    // Illustrate that the same helper can iterate over any bodies vector.
    // -------------------------------------------------------------------------
    std::cout << "--- Bodies visible (>20°) without the Sun ---\n\n";
    std::vector<std::unique_ptr<Target>> night_bodies;
    night_bodies.push_back(std::make_unique<BodyTarget>(Body::Moon));
    night_bodies.push_back(std::make_unique<StarTarget>(SIRIUS));
    night_bodies.push_back(std::make_unique<StarTarget>(VEGA));
    night_bodies.push_back(std::make_unique<StarTarget>(CANOPUS));
    night_bodies.push_back(std::make_unique<StarTarget>(RIGEL));
    night_bodies.push_back(std::make_unique<StarTarget>(ALTAIR));
    night_bodies.push_back(std::make_unique<StarTarget>(ARCTURUS));
    night_bodies.push_back(std::make_unique<StarTarget>(ALDEBARAN));

    for (const auto &b : night_bodies) {
        const auto s = analyze_body(*b, observer, window, threshold_deg);
        print_summary(s);
    }

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
