/**
 * @file demo.cpp
 * @example demo.cpp
 * @brief End-to-end demo of siderust-cpp extended capabilities.
 */

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

#include <siderust/siderust.hpp>

namespace {

const char* crossing_direction_name(siderust::CrossingDirection dir) {
    using siderust::CrossingDirection;
    switch (dir) {
    case CrossingDirection::Rising:
        return "rising";
    case CrossingDirection::Setting:
        return "setting";
    }
    return "unknown";
}

const char* moon_phase_label_name(siderust::MoonPhaseLabel label) {
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

    const Geodetic site = ROQUE_DE_LOS_MUCHACHOS;
    const JulianDate jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
    const MJD now = MJD::from_jd(jd);
    const Period next_day(now, now + qtty::Day(1.0));

    std::cout << "=== siderust-cpp extended demo ===\n";
    std::cout << "Observer: lon=" << site.lon.value()
              << " deg lat=" << site.lat.value()
              << " deg h=" << site.height.value() << " m\n";
    std::cout << "Epoch: JD " << std::fixed << std::setprecision(6) << jd.value()
              << "  UTC " << jd.to_utc() << "\n\n";

    spherical::direction::ICRS vega_icrs(279.23473, 38.78369);
    auto vega_ecl = vega_icrs.to_frame<frames::EclipticMeanJ2000>(jd);
    auto vega_hor = vega_icrs.to_horizontal(jd, site);
    std::cout << "Typed coordinates\n";
    std::cout << "  Vega ICRS   RA=" << vega_icrs.ra().value()
              << " deg  Dec=" << vega_icrs.dec().value() << " deg\n";
    std::cout << "  Vega Ecliptic lon=" << vega_ecl.lon().value()
              << " deg  lat=" << vega_ecl.lat().value() << " deg\n";
    std::cout << "  Vega Horizontal az=" << vega_hor.az().value()
              << " deg  alt=" << vega_hor.alt().value() << " deg\n\n";

    qtty::Degree sun_alt = sun::altitude_at(site, now).to<qtty::Degree>();
    qtty::Degree sun_az  = sun::azimuth_at(site, now);
    std::cout << "Sun instant\n";
    std::cout << "  Altitude=" << sun_alt.value() << " deg"
              << "  Azimuth=" << sun_az.value() << " deg\n";

    auto sun_crossings = sun::crossings(site, next_day, qtty::Degree(0.0));
    if (!sun_crossings.empty()) {
        std::cout << "  Next horizon crossing: "
                  << sun_crossings.front().time.to_utc() << " ("
                  << crossing_direction_name(sun_crossings.front().direction)
                  << ")\n";
    }
    std::cout << "\n";

    BodyTarget mars(Body::Mars);
    Target fixed_target(279.23473, 38.78369); // Vega-like fixed ICRS pointing

    std::vector<std::pair<const char*, std::unique_ptr<Trackable>>> targets;
    targets.push_back({"Sun", std::make_unique<BodyTarget>(Body::Sun)});
    targets.push_back({"Vega", std::make_unique<StarTarget>(VEGA)});
    targets.push_back({"Fixed target", std::make_unique<Target>(279.23473, 38.78369)});

    std::cout << "Trackable polymorphism\n";
    for (const auto& entry : targets) {
        const auto& name = entry.first;
        const auto& obj  = entry.second;
        auto alt = obj->altitude_at(site, now);
        auto az  = obj->azimuth_at(site, now);
        std::cout << "  " << std::setw(12) << std::left << name
                  << " alt=" << std::setw(8) << alt.value()
                  << " deg  az=" << az.value() << " deg\n";
    }
    std::cout << "  Mars altitude via BodyTarget: "
              << mars.altitude_at(site, now).value() << " deg\n";
    std::cout << "  Fixed Target altitude: "
              << fixed_target.altitude_at(site, now).value() << " deg\n\n";

    auto earth_helio = ephemeris::earth_heliocentric(jd);
    auto moon_geo    = ephemeris::moon_geocentric(jd);
    double moon_dist_km = std::sqrt(
        moon_geo.x().value() * moon_geo.x().value() +
        moon_geo.y().value() * moon_geo.y().value() +
        moon_geo.z().value() * moon_geo.z().value());

    std::cout << "Ephemeris\n";
    std::cout << "  Earth heliocentric x=" << earth_helio.x().value()
              << " AU y=" << earth_helio.y().value() << " AU\n";
    std::cout << "  Moon geocentric distance=" << moon_dist_km << " km\n\n";

    auto phase = moon::phase_topocentric(jd, site);
    auto label = moon::phase_label(phase);
    auto bright_periods = moon::illumination_above(
        Period(now, now + qtty::Day(7.0)), 0.8);

    std::cout << "Lunar phase\n";
    std::cout << "  Illuminated fraction=" << phase.illuminated_fraction
              << "  label=" << moon_phase_label_name(label) << "\n";
    std::cout << "  Bright-moon periods (next 7 days, k>=0.8): "
              << bright_periods.size() << "\n";

    return 0;
}
