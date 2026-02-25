/**
 * @file trackable_targets_example.cpp
 * @example trackable_targets_example.cpp
 * @brief Using Target<C>, StarTarget, BodyTarget through Trackable polymorphism.
 *
 * Demonstrates the strongly-typed Target template with multiple frames:
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

namespace {

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

    // Strongly-typed ICRS target — ra() / dec() return qtty::Degree.
    ICRSTarget fixed_vega_like{ spherical::direction::ICRS{
        qtty::Degree(279.23473), qtty::Degree(38.78369) } };
    std::cout << "ICRSTarget metadata\n";
    std::cout << "  RA/Dec=" << fixed_vega_like.direction()
              << "  epoch=" << fixed_vega_like.epoch() << " JD\n\n";

    // Ecliptic target (Vega in EclipticMeanJ2000, lon≈279.6°, lat≈+61.8°).
    // Automatically converted to ICRS by the constructor.
    EclipticMeanJ2000Target ecliptic_vega{ spherical::direction::EclipticMeanJ2000{
        qtty::Degree(279.6), qtty::Degree(61.8) } };
    auto alt_ecliptic = ecliptic_vega.altitude_at(site, now);
    std::cout << "EclipticMeanJ2000Target (Vega approx)\n";
    std::cout << "  ecl lon/lat=" << ecliptic_vega.direction() << "\n";
    std::cout << "  ICRS ra/dec=" << ecliptic_vega.icrs_direction() << " (converted)\n";
    std::cout << "  alt=" << alt_ecliptic << std::endl;

    std::vector<NamedTrackable> catalog;
    catalog.push_back({"Sun", std::make_unique<BodyTarget>(Body::Sun)});
    catalog.push_back({"Mars", std::make_unique<BodyTarget>(Body::Mars)});
    catalog.push_back({"Vega (StarTarget)", std::make_unique<StarTarget>(VEGA)});
    catalog.push_back({"Fixed Vega-like (ICRS)", std::make_unique<ICRSTarget>(
        spherical::direction::ICRS{ qtty::Degree(279.23473), qtty::Degree(38.78369) })});

    for (const auto& entry : catalog) {
        const auto& t = entry.object;
        auto alt = t->altitude_at(site, now);
        auto az  = t->azimuth_at(site, now);

        std::cout << std::left << std::setw(22) << entry.name << std::right
                  << " alt=" << std::setw(9) << alt
                  << " az=" << az << std::endl;

        auto crossings = t->crossings(site, window, qtty::Degree(0.0));
        if (!crossings.empty()) {
            const auto& first = crossings.front();
            std::cout << "  first horizon crossing: " << first.time.to_utc()
                      << " (" << first.direction << ")\n";
        }

        auto az_cross = t->azimuth_crossings(site, window, qtty::Degree(180.0));
        if (!az_cross.empty()) {
            std::cout << "  first az=180 crossing: " << az_cross.front().time.to_utc() << "\n";
        }
    }

    return 0;
}
