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

} // namespace

int main() {
    using namespace siderust;

    const Geodetic site = ROQUE_DE_LOS_MUCHACHOS;
    const JulianDate jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
    const MJD now = MJD::from_jd(jd);
    const Period next_day(now, now + qtty::Day(1.0));

    std::cout << "=== siderust-cpp extended demo ===\n";
    std::cout << "Observer: " << site << "\n";
    std::cout << "Epoch: JD " << std::fixed << std::setprecision(6) << jd.value()
              << "  UTC " << jd.to_utc() << "\n\n";

    spherical::direction::ICRS vega_icrs(qtty::Degree(279.23473), qtty::Degree(38.78369));
    auto vega_ecl = vega_icrs.to_frame<frames::EclipticMeanJ2000>(jd);
    auto vega_hor = vega_icrs.to_horizontal(jd, site);
    std::cout << "Typed coordinates\n";
    std::cout << "  Vega ICRS   RA/Dec=" << vega_icrs << " deg\n";
    std::cout << "  Vega Ecliptic lon/lat=" << vega_ecl << " deg\n";
    std::cout << "  Vega Horizontal az/alt=" << vega_hor << " deg\n\n";

    qtty::Degree sun_alt = sun::altitude_at(site, now).to<qtty::Degree>();
    qtty::Degree sun_az  = sun::azimuth_at(site, now);
    std::cout << "Sun instant\n";
    std::cout << "  Altitude=" << sun_alt.value() << " deg"
              << "  Azimuth=" << sun_az.value() << " deg\n";

    auto sun_crossings = sun::crossings(site, next_day, qtty::Degree(0.0));
    if (!sun_crossings.empty()) {
        std::cout << "  Next horizon crossing: "
                  << sun_crossings.front().time.to_utc() << " ("
                  << sun_crossings.front().direction
                  << ")\n";
    }
    std::cout << "\n";

    BodyTarget mars(Body::Mars);
    ICRSTarget fixed_target{ spherical::direction::ICRS{
        qtty::Degree(279.23473), qtty::Degree(38.78369) } }; // Vega-like

    std::vector<std::pair<const char*, std::unique_ptr<Trackable>>> targets;
    targets.push_back({"Sun", std::make_unique<BodyTarget>(Body::Sun)});
    targets.push_back({"Vega", std::make_unique<StarTarget>(VEGA)});
    targets.push_back({"Fixed target", std::make_unique<ICRSTarget>(
        spherical::direction::ICRS{ qtty::Degree(279.23473), qtty::Degree(38.78369) })});

    std::cout << "Trackable polymorphism\n";
    for (const auto& entry : targets) {
        const auto& name = entry.first;
        const auto& obj  = entry.second;
        auto alt = obj->altitude_at(site, now);
        auto az  = obj->azimuth_at(site, now);
        std::cout << "  " << std::setw(12) << std::left << name
                  << " alt=" << std::setw(8) << alt
                  << " az=" << az << std::endl;
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
    std::cout << "  Earth heliocentric " << earth_helio << " AU\n";
    std::cout << "  Moon geocentric distance=" << moon_dist_km << " km\n\n";

    auto phase = moon::phase_topocentric(jd, site);
    auto label = moon::phase_label(phase);
    auto bright_periods = moon::illumination_above(
        Period(now, now + qtty::Day(7.0)), 0.8);

    std::cout << "Lunar phase\n";
    std::cout << "  Illuminated fraction=" << phase.illuminated_fraction
              << "  label=" << label << "\n";
    std::cout << "  Bright-moon periods (next 7 days, k>=0.8): "
              << bright_periods.size() << "\n";

    return 0;
}
