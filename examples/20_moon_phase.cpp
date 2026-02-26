/**
 * @file 20_moon_phase.cpp
 * @brief C++ port of siderust/examples/34_moon_phase.rs
 *
 * Demonstrates the full lunar-phase API: geocentric/topocentric geometry,
 * phase labels, phase-event search, and illumination-above queries.
 *
 * Run with: cmake --build build --target moon_phase_example
 */

#include <cmath>
#include <iomanip>
#include <iostream>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

static const char *phase_kind_str(PhaseKind k) {
    switch (k) {
    case PhaseKind::NewMoon:      return "New Moon";
    case PhaseKind::FirstQuarter: return "First Quarter";
    case PhaseKind::FullMoon:     return "Full Moon";
    case PhaseKind::LastQuarter:  return "Last Quarter";
    default:                      return "Unknown";
    }
}

static const char *phase_label_str(MoonPhaseLabel l) {
    switch (l) {
    case MoonPhaseLabel::NewMoon:        return "New Moon";
    case MoonPhaseLabel::WaxingCrescent: return "Waxing Crescent";
    case MoonPhaseLabel::FirstQuarter:   return "First Quarter";
    case MoonPhaseLabel::WaxingGibbous:  return "Waxing Gibbous";
    case MoonPhaseLabel::FullMoon:       return "Full Moon";
    case MoonPhaseLabel::WaningGibbous:  return "Waning Gibbous";
    case MoonPhaseLabel::LastQuarter:    return "Last Quarter";
    case MoonPhaseLabel::WaningCrescent: return "Waning Crescent";
    default:                             return "Unknown";
    }
}

static void print_geometry(const char *label, const MoonPhaseGeometry &g) {
    const double phase_angle_deg = g.phase_angle_rad * 180.0 / M_PI;
    std::cout << std::left << std::setw(22) << label
              << "  phase=" << std::right << std::setw(7) << std::fixed
              << std::setprecision(2) << phase_angle_deg << "°"
              << "  illum=" << std::setw(6) << std::setprecision(3)
              << g.illuminated_fraction
              << "  waxing=" << (g.waxing ? "yes" : "no")
              << "\n";
}

int main() {
    std::cout << "\u2554\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\n";
    std::cout << "\u2551              Lunar Phase Analysis                    \u2551\n";
    std::cout << "\u255a\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\n\n";

    // -------------------------------------------------------------------------
    // Part 1: Geocentric phase geometry at J2000.0
    // -------------------------------------------------------------------------
    std::cout << "--- Part 1: Phase Geometry at Key Epochs ---\n\n";

    // Sample seven epochs spanning two weeks
    const JulianDate jd0 = JulianDate::J2000();
    for (int d = 0; d <= 28; d += 4) {
        const JulianDate jd(jd0.value() + d);
        const auto geo = moon::phase_geocentric(jd);
        const auto lbl = moon::phase_label(geo);
        char buf[48];
        std::snprintf(buf, sizeof(buf), "JD+%2d days", d);
        print_geometry(buf, geo);
        std::cout << "  → label: " << phase_label_str(lbl) << "\n";
    }
    std::cout << "\n";

    // -------------------------------------------------------------------------
    // Part 2: Topocentric phase at Roque de los Muchachos
    // -------------------------------------------------------------------------
    std::cout << "--- Part 2: Geocentric vs Topocentric Phase (slight difference) ---\n\n";

    const Geodetic obs = ROQUE_DE_LOS_MUCHACHOS;
    const JulianDate jd_full(2451545.0 + 14.0); // roughly full moon region

    const auto geo_geom  = moon::phase_geocentric(jd_full);
    const auto topo_geom = moon::phase_topocentric(jd_full, obs);

    print_geometry("Geocentric",  geo_geom);
    print_geometry("Topocentric", topo_geom);

    const double diff = std::abs(geo_geom.illuminated_fraction - topo_geom.illuminated_fraction);
    std::cout << "  illumination difference: " << std::scientific << std::setprecision(3)
              << diff << "\n\n";

    // -------------------------------------------------------------------------
    // Part 3: Find all principal phase events in 3 months
    // -------------------------------------------------------------------------
    std::cout << "--- Part 3: Phase Events Over 3 Months ---\n\n";

    const Period quarter(MJD(60000.0), MJD(60090.0));
    const auto events = moon::find_phase_events(quarter);

    std::cout << "Found " << events.size() << " phase events:\n";
    for (const auto &ev : events) {
        std::cout << "  MJD=" << std::fixed << std::setprecision(2) << ev.time.value()
                  << "  → " << phase_kind_str(ev.kind) << "\n";
    }
    std::cout << "\n";

    // -------------------------------------------------------------------------
    // Part 4: Illumination above threshold (bright moon periods)
    // -------------------------------------------------------------------------
    std::cout << "--- Part 4: Illumination ≥ 50% (bright moon periods) ---\n\n";

    const auto bright_periods = moon::illumination_above(quarter, 0.50);
    double total_bright_days = 0.0;
    for (const auto &p : bright_periods) {
        const double d = p.duration<qtty::Day>().value();
        total_bright_days += d;
        std::cout << "  MJD " << std::setprecision(2) << p.start().value()
                  << " – " << p.end().value()
                  << "  (" << std::setprecision(1) << d << " d)\n";
    }
    std::cout << "  Total bright days: " << std::setprecision(1) << total_bright_days
              << " / 90 d\n\n";

    // -------------------------------------------------------------------------
    // Part 5: Moon-free (illumination < 5%) for dark-sky scheduling
    // -------------------------------------------------------------------------
    std::cout << "--- Part 5: Near-Dark Moon Windows (illumination < 5%) ---\n\n";

    const auto dark_moon_periods = moon::illumination_above(quarter, 0.01);
    // Invert: total window minus illumination above 1%
    // (Simplified: just report dark periods count and duration from above search)
    const auto very_dark_periods = moon::illumination_above(quarter, 0.05);
    std::cout << "Periods with illumination > 5%: " << very_dark_periods.size() << "\n";
    std::cout << "(Complement = dark-moon windows, ideal for deep-sky observing)\n";

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
