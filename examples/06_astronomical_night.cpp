/**
 * @file 06_astronomical_night.cpp
 * @brief C++ port of siderust/examples/25_astronomical_night.rs
 *
 * Demonstrates finding astronomical night periods using the siderust library.
 * Astronomical night is defined as the period when the Sun's center is
 * more than 18° below the horizon (altitude < -18°).
 *
 * Usage:
 *   ./06_astronomical_night [YYYY-MM-DD] [lat_deg] [lon_deg] [height_m]
 *
 * Defaults:
 *   - Start date: MJD 60000.0 (2023-02-25)
 *   - Location: Greenwich Observatory (51.4769°N, 0°E)
 *   - Search period: 7 days
 *
 * Run with: cmake --build build --target astronomical_night_example
 */

#include <cstdlib>
#include <iomanip>
#include <iostream>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

int main(int argc, char *argv[]) {
    // Parse optional arguments (lon, lat, height, start_mjd)
    const double lat_deg  = argc > 1 ? std::atof(argv[1]) : 51.4769;
    const double lon_deg  = argc > 2 ? std::atof(argv[2]) :  0.0;
    const double height_m = argc > 3 ? std::atof(argv[3]) :  0.0;
    const double mjd0     = argc > 4 ? std::atof(argv[4]) : 60000.0;

    const Geodetic site = Geodetic(lon_deg, lat_deg, height_m);
    const MJD start(mjd0);
    const MJD end(mjd0 + 7.0);   // 7-day window
    const Period window(start, end);

    std::cout << "Astronomical Night Periods (Sun altitude < -18°)\n";
    std::cout << "================================================\n";
    std::cout << "Observer: lat = " << lat_deg  << "°, "
              << "lon = " << lon_deg  << "°, "
              << "height = " << height_m << " m\n";
    std::cout << "MJD window: " << start.value() << " → " << end.value()
              << " (7 days)\n\n";

    // Find astronomical night periods (Sun < -18°)
    const auto nights = sun::below_threshold(site, window, qtty::Degree(-18.0));

    if (nights.empty()) {
        std::cout << "No astronomical night periods found in this week.\n";
        std::cout << "(This can happen at high latitudes during summer.)\n";
    } else {
        std::cout << "Found " << nights.size() << " astronomical night period(s):\n\n";
        for (const auto &period : nights) {
            const double dur_min = period.duration<qtty::Minute>().value();
            std::cout << "  MJD " << std::fixed << std::setprecision(4)
                      << period.start().value()
                      << " → " << period.end().value()
                      << "  (" << std::setprecision(1) << dur_min << " min)\n";
        }
    }

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
