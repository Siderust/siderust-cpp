// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// Star observability in altitude + azimuth ranges.
///
/// Build & run:
///   cmake --build build-local --target 09_star_observability_example
///   ./build-local/09_star_observability_example

#include <siderust/siderust.hpp>

#include <algorithm>
#include <iomanip>
#include <iostream>

using namespace siderust;
using namespace qtty::literals;

/// Intersect two sorted vectors of periods.
/// Returns every non-empty overlap between a period in `a` and a period in `b`.
static std::vector<Period> intersect_periods(const std::vector<Period> &a,
                                             const std::vector<Period> &b) {
    std::vector<Period> result;
    size_t j = 0;
    for (size_t i = 0; i < a.size() && j < b.size(); ) {
        double lo = std::max(a[i].start().value(), b[j].start().value());
        double hi = std::min(a[i].end().value(),   b[j].end().value());
        if (lo < hi) {
            result.push_back(Period(MJD(lo), MJD(hi)));
        }
        // advance whichever period ends first
        if (a[i].end().value() < b[j].end().value()) ++i; else ++j;
    }
    return result;
}

int main() {
    std::cout << "Star observability: altitude + azimuth constraints\n" << std::endl;

    const auto &observer = ROQUE_DE_LOS_MUCHACHOS;
    const auto &target   = SIRIUS;

    // One-night search window (MJD TT).
    MJD t0(60000.0);
    Period window(t0, t0 + 1.0_d);

    // Constraint 1: altitude between 25° and 65°.
    auto min_alt = 25.0_deg;
    auto max_alt = 65.0_deg;
    auto above_min = star_altitude::above_threshold(target, observer, window, min_alt);
    auto below_max = star_altitude::below_threshold(target, observer, window, max_alt);
    auto altitude_periods = intersect_periods(above_min, below_max);

    // Constraint 2: azimuth between 110° and 220° (ESE -> SW sector).
    auto min_az = 110.0_deg;
    auto max_az = 220.0_deg;
    auto azimuth_periods = star_altitude::in_azimuth_range(
        target, observer, window, min_az, max_az);

    // Final observability: periods satisfying both constraints simultaneously.
    auto observable = intersect_periods(altitude_periods, azimuth_periods);

    std::cout << "Observer: Roque de los Muchachos" << std::endl;
    std::cout << "Target: Sirius" << std::endl;
    std::cout << "Window: MJD " << std::fixed << std::setprecision(1)
              << window.start() << " -> " << window.end()
              << "\n" << std::endl;

    std::cout << "Altitude range: " << min_alt << " .. " << max_alt << std::endl;
    std::cout << "Azimuth range:  " << min_az << " .. " << max_az << "\n" << std::endl;

    std::cout << "Matched periods: " << observable.size() << std::endl;
    double total_hours = 0.0;
    for (size_t i = 0; i < observable.size(); ++i) {
        auto hours = observable[i].duration<qtty::Hour>();
        total_hours += hours.value();
        std::cout << "  " << (i + 1) << ". MJD "
                  << std::fixed << std::setprecision(6)
                  << observable[i].start() << " -> "
                  << observable[i].end()
                  << "  (" << std::setprecision(4) << hours << ")"
                  << std::endl;
    }

    std::cout << "\nTotal observable time in both ranges: "
              << std::setprecision(4) << qtty::Hour(total_hours) << std::endl;

    return 0;
}
