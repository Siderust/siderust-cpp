// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// Night Events Example
///
/// Shows how to spot "night-type" crossing events and night periods in a
/// one-week window using civil/nautical/astronomical/horizon thresholds.
///
/// Build & run:
///   cmake --build build-local --target 06_night_events_example
///   ./build-local/06_night_events_example [lat_deg] [lon_deg] [height_m]

#include <siderust/siderust.hpp>

#include <iomanip>
#include <iostream>
#include <vector>

using namespace siderust;
using namespace qtty::literals;

// Twilight threshold constants (same values as siderust::calculus::solar::night_types)
namespace twilight {
constexpr auto HORIZON           = 0.0_deg;
constexpr auto APPARENT_HORIZON  = qtty::Degree(-0.833);
constexpr auto CIVIL             = qtty::Degree(-6.0);
constexpr auto NAUTICAL          = qtty::Degree(-12.0);
constexpr auto ASTRONOMICAL      = qtty::Degree(-18.0);
} // namespace twilight

static Period week_from_mjd(const MJD &start) {
    MJD end = start + qtty::Day(7.0);
    return Period(start, end);
}

static void print_events_for_type(const Geodetic &site, const Period &week,
                                  const char *name, qtty::Degree threshold) {
    auto events = sun::crossings(site, week, threshold);
    int downs = 0, raises = 0;

    std::cout << std::left << std::setw(18) << name << " threshold "
              << std::right << std::fixed << std::setprecision(3)
              << threshold << " -> " << events.size()
              << " crossing(s)" << std::endl;

    for (auto &ev : events) {
        const char *label;
        if (ev.direction == CrossingDirection::Setting) {
            ++downs;
            label = "night-type down (Sun setting below threshold)";
        } else {
            ++raises;
            label = "night-type raise (Sun rising above threshold)";
        }
        auto utc = ev.time.to_utc();
        std::cout << "  - " << label << " at " << utc << std::endl;
    }
    std::cout << "  summary: down=" << downs << " raise=" << raises << std::endl;
}

static void print_periods_for_type(const Geodetic &site, const Period &week,
                                   const char *name, qtty::Degree threshold) {
    auto periods = sun::below_threshold(site, week, threshold);
    std::cout << std::left << std::setw(18) << name
              << " night periods (Sun < " << std::fixed << std::setprecision(3)
              << threshold << "): " << periods.size() << std::endl;

    for (auto &p : periods) {
        auto s = p.start().to_utc();
        auto e = p.end().to_utc();
        auto hours = p.duration<qtty::Hour>();
        std::cout << "  - " << s << " -> " << e
                  << " (" << std::setprecision(1) << hours << ")" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    double lat_deg  = argc > 1 ? std::stod(argv[1]) : 51.4769;
    double lon_deg  = argc > 2 ? std::stod(argv[2]) : 0.0;
    double height_m = argc > 3 ? std::stod(argv[3]) : 0.0;

    Geodetic site{qtty::Degree(lon_deg), qtty::Degree(lat_deg),
                  qtty::Meter(height_m)};

    // Fixed start date: 2024-06-01 00:00 UTC  (MJD ≈ 60461)
    auto mjd_start = MJD::from_utc({2024, 6, 1, 0, 0, 0});
    auto week = week_from_mjd(mjd_start);

    struct NightType {
        const char *name;
        qtty::Degree threshold;
    };
    NightType night_types[] = {
        {"Horizon",          twilight::HORIZON},
        {"Apparent Horizon", twilight::APPARENT_HORIZON},
        {"Civil",            twilight::CIVIL},
        {"Nautical",         twilight::NAUTICAL},
        {"Astronomical",     twilight::ASTRONOMICAL},
    };

    std::cout << "Night events over one week" << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << "Site: lat=" << lat_deg << " lon=" << lon_deg
              << " height=" << height_m << std::endl;
    std::cout << "Week start: 2024-06-01 UTC\n" << std::endl;

    std::cout << "1) Night-type crossing events" << std::endl;
    for (auto &nt : night_types) {
        print_events_for_type(site, week, nt.name, nt.threshold);
    }

    std::cout << "\n2) Night periods per night type" << std::endl;
    for (auto &nt : night_types) {
        print_periods_for_type(site, week, nt.name, nt.threshold);
    }

    return 0;
}
