/**
 * @file coordinate_systems_example.cpp
 * @example coordinate_systems_example.cpp
 * @brief Coordinate systems and frame transform walkthrough.
 *
 * Usage:
 *   cmake --build build-make --target coordinate_systems_example
 *   ./build-make/coordinate_systems_example
 */

#include <siderust/siderust.hpp>

#include <iostream>
#include <iomanip>

int main() {
    using namespace siderust;
    using namespace qtty::literals;

    std::cout << "=== coordinate_systems_example===\n";

    auto obs = ROQUE_DE_LOS_MUCHACHOS;

    std::cout << "Observer lon=" << std::fixed << std::setprecision(4) << obs.lon
              << " lat=" << obs.lat << "\n";

    return 0;
}
