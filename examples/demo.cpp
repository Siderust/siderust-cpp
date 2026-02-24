/**
 * @file demo.cpp
 * @example demo.cpp
 * @brief Demonstrates the siderust C++ API.
 *
 * Usage:
 *   cd build && cmake .. && cmake --build . && ./demo
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <siderust/siderust.hpp>

int main() {
    using namespace siderust;
    using namespace qtty::literals;

    std::cout << "=== siderust-cpp demo (concise) ===\n";

    // Time
    auto jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
    std::cout << "JD=" << std::fixed << std::setprecision(6) << jd.value() << "  UTC=" << jd.to_utc() << "\n";

    // Sun altitude (deg)
    auto mjd = MJD::from_jd(jd);
    qtty::Radian sun_alt = sun::altitude_at(ROQUE_DE_LOS_MUCHACHOS, mjd);
    std::cout << "Sun alt=" << std::fixed << std::setprecision(2) << sun_alt.to<qtty::Degree>().value() << " deg\n";

    // Vega altitude
    const auto& vega = VEGA;
    auto vega_alt = star_altitude::altitude_at(vega, ROQUE_DE_LOS_MUCHACHOS, mjd);
    std::cout << "Vega alt=" << std::setprecision(2) << vega_alt.to<qtty::Degree>().value() << " deg\n";

    // Simple ephemeris values
    auto earth = ephemeris::earth_heliocentric(jd);
    std::cout << "Earth (AU) x=" << std::setprecision(6) << earth.x().value() << " y=" << earth.y().value() << "\n";

    auto moon = ephemeris::moon_geocentric(jd);
    double moon_r = std::sqrt(moon.x().value() * moon.x().value() + moon.y().value() * moon.y().value() + moon.z().value() * moon.z().value());
    std::cout << "Moon dist=" << std::fixed << std::setprecision(2) << moon_r << " km\n";

    std::cout << "Done.\n";
    return 0;
}
