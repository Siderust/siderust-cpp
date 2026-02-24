/**
 * @file solar_system_bodies_example.cpp
 * @example solar_system_bodies_example.cpp
 * @brief Solar-system body ephemeris and catalog examples.
 *
 * Usage:
 *   cmake --build build-make --target solar_system_bodies_example
 *   ./build-make/solar_system_bodies_example
 */

#include <siderust/siderust.hpp>
#include <iostream>
#include <iomanip>
#include <cmath>

int main() {
    using namespace siderust;
    using namespace qtty::literals;

    std::cout << "=== solar_system_bodies_example (concise) ===\n";

    auto jd = JulianDate::from_utc({2026, 7, 15, 0, 0, 0});
    std::cout << "Epoch JD=" << std::fixed << std::setprecision(6) << jd.value() << "\n";

    auto earth_helio = ephemeris::earth_heliocentric(jd);
    std::cout << "Earth heliocentric x=" << std::setprecision(6) << earth_helio.x().value() << " AU\n";

    auto moon_geo = ephemeris::moon_geocentric(jd);
    double moon_dist = std::sqrt(moon_geo.x().value() * moon_geo.x().value() + moon_geo.y().value() * moon_geo.y().value() + moon_geo.z().value() * moon_geo.z().value());
    std::cout << "Moon dist=" << std::fixed << std::setprecision(2) << moon_dist << " km\n";

    // Print a couple of planets concisely
    std::cout << "Mercury a=" << MERCURY.orbit.semi_major_axis_au << " AU  Earth a=" << EARTH.orbit.semi_major_axis_au << " AU\n";

    return 0;
}
