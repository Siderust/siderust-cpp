/**
 * @file 07_find_night_periods_365day.cpp
 * @brief C++ port of siderust/examples/31_find_night_periods_365day.rs
 *
 * Runs the astronomical night finder over a full 365-day horizon and prints
 * all astronomical night periods (Sun altitude < -18°) for the default site
 * (Roque de los Muchachos Observatory, La Palma).
 *
 * Usage:
 *   ./07_find_night_periods_365day [start_mjd]
 *
 * Default: MJD 60339.0 (~2026-01-01)
 *
 * Run with: cmake --build build --target find_night_periods_365day_example
 */

#include <cstdlib>
#include <iomanip>
#include <iostream>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

int main(int argc, char *argv[]) {
    // Default: MJD for 2026-01-01
    const double start_mjd = argc > 1 ? std::atof(argv[1]) : 60339.0;

    const MJD start(start_mjd);
    const MJD end(start_mjd + 365.0);
    const Period year_window(start, end);

    std::cout << "Find astronomical night periods for 365 days starting MJD "
              << std::fixed << std::setprecision(0) << start_mjd << "\n";
    std::cout << "Observer: Roque de los Muchachos Observatory (La Palma)\n\n";

    const auto nights = sun::below_threshold(ROQUE_DE_LOS_MUCHACHOS, year_window,
                                             qtty::Degree(-18.0));

    if (nights.empty()) {
        std::cout << "No astronomical night periods found for this year at this site.\n";
        return 0;
    }

    std::cout << "Found " << nights.size() << " night periods:\n\n";
    for (const auto &p : nights) {
        const double dur_min = p.duration<qtty::Minute>().value();
        std::cout << "  MJD " << std::setprecision(4) << p.start().value()
                  << " → " << p.end().value()
                  << "  (" << std::setprecision(1) << dur_min << " min)\n";
    }

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
