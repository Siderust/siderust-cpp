/**
 * @file coordinates_examples.cpp
 * @example coordinates_examples.cpp
 * @brief Focused examples for creating and converting typed coordinates.
 */
#include <siderust/siderust.hpp>

#include <iostream>
#include <iomanip>

int main() {
    using namespace siderust;
    using namespace qtty::literals;

    std::cout << "=== coordinates_examples (concise) ===\n";

    // Geodetic -> ECEF (single line)
    Geodetic obs(-17.8890, 28.7610, 2396.0);
    auto ecef = obs.to_cartesian();
    std::cout << "Geodetic lon=" << std::fixed << std::setprecision(4) << obs.lon.value()
              << " lat=" << obs.lat.value() << " h=" << obs.height.value() << " m\n";

    // Spherical direction example (ICRS -> horizontal)
    spherical::direction::ICRS vega_icrs(279.23473, 38.78369);
    auto jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
    auto hor = vega_icrs.to_horizontal(jd, obs);
    std::cout << "Vega az=" << std::setprecision(2) << hor.az().value() << " alt=" << hor.alt().value() << "\n";

    // Ephemeris quick values
    auto earth = ephemeris::earth_heliocentric(jd);
    std::cout << "Earth x=" << std::setprecision(6) << earth.x().value() << " AU\n";

    std::cout << "Done.\n";
    return 0;
}
