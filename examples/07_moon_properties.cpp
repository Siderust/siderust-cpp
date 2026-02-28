// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// Moon phase quick examples.
///
/// Shows:
/// 1) how to get moon phase properties at a given instant,
/// 2) how to find windows where illumination is in a given range.
///
/// Build & run: cmake --build build-local --target 07_moon_properties_example
///              ./build-local/07_moon_properties_example

#include <siderust/siderust.hpp>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace siderust;

/// Helper: print a list of MJD periods with their durations.
void print_periods(const std::string &label,
                   const std::vector<Period> &periods) {
    std::cout << "\n" << label << ": " << periods.size() << " period(s)"
              << std::endl;
    for (const auto &p : periods) {
        auto dur_h = p.duration<qtty::Hour>();
        auto s = p.start().to_utc();
        auto e = p.end().to_utc();
        std::cout << "  - " << s << " -> " << e << " (" << dur_h << ")"
                  << std::endl;
    }
}

int main() {
    // Default site: Roque de los Muchachos
    double lat = 28.762;
    double lon = -17.892;
    double h_m = 2396.0;

    Geodetic site{qtty::Degree(lon), qtty::Degree(lat), qtty::Meter(h_m)};

    // Use a fixed date for reproducibility: 2026-03-01 00:00 UTC
    auto jd = JulianDate::from_utc({2026, 3, 1, 0, 0, 0});
    auto mjd = jd.to<tempoch::MJDScale>();
    auto window = Period(mjd, MJD(mjd.value() + 35.0));
    SearchOptions opts{};

    // =========================================================================
    // 1) Point-in-time phase properties
    // =========================================================================
    auto geo = moon::phase_geocentric(jd);
    auto topo = moon::phase_topocentric(jd, site);

    constexpr double RAD_TO_DEG = 180.0 / M_PI;

    std::cout << std::fixed;
    std::cout << "Moon phase at 2026-03-01 00:00 UTC" << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << std::setprecision(4);
    std::cout << "Site: lat=" << lat << " deg, lon=" << lon
              << " deg, h=" << std::setprecision(0) << h_m << " m" << std::endl;

    std::cout << "\nGeocentric:" << std::endl;
    std::cout << "  label                 : " << moon::phase_label(geo) << std::endl;
    std::cout << std::setprecision(4);
    std::cout << "  illuminated fraction  : " << geo.illuminated_fraction << std::endl;
    std::cout << std::setprecision(2);
    std::cout << "  illuminated percent   : " << illuminated_percent(geo) << " %"
              << std::endl;
    std::cout << "  phase angle           : "
              << geo.phase_angle_rad * RAD_TO_DEG << " deg" << std::endl;
    std::cout << "  elongation            : "
              << geo.elongation_rad * RAD_TO_DEG << " deg" << std::endl;
    std::cout << "  waxing                : " << std::boolalpha << geo.waxing
              << std::endl;

    std::cout << "\nTopocentric:" << std::endl;
    std::cout << "  label                 : " << moon::phase_label(topo)
              << std::endl;
    std::cout << std::setprecision(4);
    std::cout << "  illuminated fraction  : " << topo.illuminated_fraction
              << std::endl;
    std::cout << "  illumination delta    : "
              << std::showpos
              << (topo.illuminated_fraction - geo.illuminated_fraction) * 100.0
              << std::noshowpos << " %" << std::endl;
    std::cout << "  elongation            : "
              << topo.elongation_rad * RAD_TO_DEG << " deg" << std::endl;

    // =========================================================================
    // 2) Principal phase events
    // =========================================================================
    auto events = moon::find_phase_events(window, opts);
    std::cout << "\nPrincipal phase events in next 35 days: " << events.size()
              << std::endl;
    for (const auto &ev : events) {
        auto utc = ev.time.to_utc();
        std::cout << "  - " << std::setw(13) << std::right << ev.kind
                  << " at " << utc << " UTC" << std::endl;
    }

    // =========================================================================
    // 3) Illumination range searches
    // =========================================================================
    auto crescent   = moon::illumination_range(window, 0.05, 0.35, opts);
    auto quarterish = moon::illumination_range(window, 0.45, 0.55, opts);
    auto gibbous    = moon::illumination_range(window, 0.65, 0.95, opts);

    print_periods("Crescent-like range (5%-35%)", crescent);
    print_periods("Quarter-like range (45%-55%)", quarterish);
    print_periods("Gibbous-like range (65%-95%)", gibbous);

    return 0;
}
