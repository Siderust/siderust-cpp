/**
 * @file altitude_events_example.cpp
 * @example altitude_events_example.cpp
 * @brief Concise altitude events example using streamed UTC and Period printing.
 */

#include <siderust/siderust.hpp>
#include <iostream>
#include <iomanip>
#include <vector>

int main() {
    using namespace siderust;
    using namespace qtty::literals;

    std::cout << "=== altitude_events_example (concise) ===\n";

    const auto obs = MAUNA_KEA;
    const auto start = MJD::from_utc({2026, 7, 15, 0, 0, 0});
    const auto end = start + qtty::Day(2.0);

    auto nights = sun::below_threshold(obs, start, end, -18.0_deg);
    std::cout << "Astronomical nights found: " << nights.size() << "\n";

    // Print up to three night periods as UTC ranges and duration in hours
    const std::size_t n = std::min<std::size_t>(nights.size(), 3);
    for (std::size_t i = 0; i < n; ++i) {
        const auto &p = nights[i];
        std::cout << "  " << (i + 1) << ") " << p.start().to_utc() << " -> " << p.end().to_utc()
                  << "  (" << std::fixed << std::setprecision(2) << p.duration<qtty::Hour>().value() << " h)\n";
    }

    auto crossings = sun::crossings(obs, start, end, 0.0_deg);
    std::cout << "Sun crossings: " << crossings.size() << "\n";

    auto culminations = sun::culminations(obs, start, end);
    std::cout << "Culminations: " << culminations.size() << "\n";

    std::cout << "Done.\n";
    return 0;
}
