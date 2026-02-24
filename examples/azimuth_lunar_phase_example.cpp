/**
 * @file azimuth_lunar_phase_example.cpp
 * @example azimuth_lunar_phase_example.cpp
 * @brief Azimuth event search plus lunar phase geometry/events.
 */

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

#include <siderust/siderust.hpp>

namespace {

const char* az_kind_name(siderust::AzimuthExtremumKind kind) {
    using siderust::AzimuthExtremumKind;
    switch (kind) {
    case AzimuthExtremumKind::Max:
        return "max";
    case AzimuthExtremumKind::Min:
        return "min";
    }
    return "unknown";
}

const char* phase_kind_name(siderust::PhaseKind kind) {
    using siderust::PhaseKind;
    switch (kind) {
    case PhaseKind::NewMoon:
        return "new moon";
    case PhaseKind::FirstQuarter:
        return "first quarter";
    case PhaseKind::FullMoon:
        return "full moon";
    case PhaseKind::LastQuarter:
        return "last quarter";
    }
    return "unknown";
}

const char* phase_label_name(siderust::MoonPhaseLabel label) {
    using siderust::MoonPhaseLabel;
    switch (label) {
    case MoonPhaseLabel::NewMoon:
        return "new moon";
    case MoonPhaseLabel::WaxingCrescent:
        return "waxing crescent";
    case MoonPhaseLabel::FirstQuarter:
        return "first quarter";
    case MoonPhaseLabel::WaxingGibbous:
        return "waxing gibbous";
    case MoonPhaseLabel::FullMoon:
        return "full moon";
    case MoonPhaseLabel::WaningGibbous:
        return "waning gibbous";
    case MoonPhaseLabel::LastQuarter:
        return "last quarter";
    case MoonPhaseLabel::WaningCrescent:
        return "waning crescent";
    }
    return "unknown";
}

} // namespace

int main() {
    using namespace siderust;

    const Geodetic site = MAUNA_KEA;
    const MJD start = MJD::from_utc({2026, 7, 15, 0, 0, 0});
    const MJD end = start + qtty::Day(3.0);
    const Period window(start, end);

    std::cout << "=== azimuth_lunar_phase_example ===\n";
    std::cout << "Window UTC: " << start.to_utc() << " -> " << end.to_utc() << "\n\n";

    const MJD now = MJD::from_utc({2026, 7, 15, 12, 0, 0});
    std::cout << "Instant azimuth\n";
    std::cout << "  Sun  : " << sun::azimuth_at(site, now).value() << " deg\n";
    std::cout << "  Moon : " << moon::azimuth_at(site, now).value() << " deg\n";
    std::cout << "  Vega : " << star_altitude::azimuth_at(VEGA, site, now).value() << " deg\n\n";

    auto sun_cross = sun::azimuth_crossings(site, window, qtty::Degree(180.0));
    auto sun_ext   = sun::azimuth_extrema(site, window);
    auto moon_west = moon::in_azimuth_range(site, window, qtty::Degree(240.0), qtty::Degree(300.0));

    std::cout << "Azimuth events\n";
    std::cout << "  Sun crossings at 180 deg: " << sun_cross.size() << "\n";
    std::cout << "  Sun azimuth extrema: " << sun_ext.size() << "\n";
    if (!sun_ext.empty()) {
        const auto& e = sun_ext.front();
        std::cout << "    first extremum " << az_kind_name(e.kind)
                  << " at " << e.time.to_utc()
                  << " az=" << e.azimuth.value() << " deg\n";
    }
    std::cout << "  Moon in [240,300] deg azimuth: " << moon_west.size() << " period(s)\n\n";

    const JulianDate jd_now = now.to_jd();
    auto geo_phase = moon::phase_geocentric(jd_now);
    auto topo_phase = moon::phase_topocentric(jd_now, site);
    auto topo_label = moon::phase_label(topo_phase);

    auto phase_events = moon::find_phase_events(
        Period(start, start + qtty::Day(30.0)));
    auto half_lit = moon::illumination_range(window, 0.45, 0.55);

    std::cout << "Lunar phase\n";
    std::cout << std::fixed << std::setprecision(3)
              << "  Geocentric illuminated fraction: " << geo_phase.illuminated_fraction << "\n"
              << "  Topocentric illuminated fraction: " << topo_phase.illuminated_fraction
              << " (" << phase_label_name(topo_label) << ")\n";

    std::cout << "  Principal phase events in next 30 days: " << phase_events.size() << "\n";
    const std::size_t n = std::min<std::size_t>(phase_events.size(), 4);
    for (std::size_t i = 0; i < n; ++i) {
        const auto& ev = phase_events[i];
        std::cout << "    " << ev.time.to_utc() << " -> " << phase_kind_name(ev.kind) << "\n";
    }

    std::cout << "  Near-half illumination periods (k in [0.45, 0.55]): "
              << half_lit.size() << "\n";

    return 0;
}
