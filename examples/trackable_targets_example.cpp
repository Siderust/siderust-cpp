/**
 * @file trackable_targets_example.cpp
 * @example trackable_targets_example.cpp
 * @brief Using Target, StarTarget, BodyTarget through Trackable polymorphism.
 */

#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <siderust/siderust.hpp>

namespace {

const char* crossing_direction_name(siderust::CrossingDirection dir) {
    using siderust::CrossingDirection;
    switch (dir) {
    case CrossingDirection::Rising:
        return "rising";
    case CrossingDirection::Setting:
        return "setting";
    }
    return "unknown";
}

struct NamedTrackable {
    std::string                        name;
    std::unique_ptr<siderust::Trackable> object;
};

} // namespace

int main() {
    using namespace siderust;

    const Geodetic site = geodetic(-17.8890, 28.7610, 2396.0);
    const MJD now = MJD::from_utc({2026, 7, 15, 22, 0, 0});
    const Period window(now, now + qtty::Day(1.0));

    std::cout << "=== trackable_targets_example ===\n";
    std::cout << "Epoch UTC: " << now.to_utc() << "\n\n";

    Target fixed_vega_like(279.23473, 38.78369);
    std::cout << "Target metadata\n";
    std::cout << "  RA=" << fixed_vega_like.ra_deg() << " deg"
              << " Dec=" << fixed_vega_like.dec_deg() << " deg"
              << " epoch JD=" << fixed_vega_like.epoch_jd() << "\n\n";

    std::vector<NamedTrackable> catalog;
    catalog.push_back({"Sun", std::make_unique<BodyTarget>(Body::Sun)});
    catalog.push_back({"Mars", std::make_unique<BodyTarget>(Body::Mars)});
    catalog.push_back({"Vega (StarTarget)", std::make_unique<StarTarget>(VEGA)});
    catalog.push_back({"Fixed Vega-like Target", std::make_unique<Target>(279.23473, 38.78369)});

    for (const auto& entry : catalog) {
        const auto& t = entry.object;
        auto alt = t->altitude_at(site, now);
        auto az  = t->azimuth_at(site, now);

        std::cout << std::left << std::setw(22) << entry.name << std::right
                  << " alt=" << std::setw(9) << alt.value() << " deg"
                  << " az=" << az.value() << " deg\n";

        auto crossings = t->crossings(site, window, qtty::Degree(0.0));
        if (!crossings.empty()) {
            const auto& first = crossings.front();
            std::cout << "  first horizon crossing: " << first.time.to_utc()
                      << " (" << crossing_direction_name(first.direction) << ")\n";
        }

        auto az_cross = t->azimuth_crossings(site, window, qtty::Degree(180.0));
        if (!az_cross.empty()) {
            std::cout << "  first az=180 crossing: " << az_cross.front().time.to_utc() << "\n";
        }
    }

    return 0;
}
