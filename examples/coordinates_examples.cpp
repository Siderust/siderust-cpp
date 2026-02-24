/**
 * @file coordinates_examples.cpp
 * @example coordinates_examples.cpp
 * @brief Focused typed-coordinate construction and conversion examples.
 */

#include <iomanip>
#include <iostream>
#include <type_traits>

#include <siderust/siderust.hpp>

int main() {
    using namespace siderust;

    std::cout << "=== coordinates_examples ===\n";

    const Geodetic site(-17.8890, 28.7610, 2396.0);
    const auto ecef_m  = site.to_cartesian();
    const auto ecef_km = site.to_cartesian<qtty::Kilometer>();

    static_assert(std::is_same_v<
                  std::remove_cv_t<decltype(ecef_m)>,
                  cartesian::position::ECEF<qtty::Meter>>);

    std::cout << "Geodetic -> ECEF\n";
    std::cout << "  lon=" << site.lon.value() << " deg lat=" << site.lat.value()
              << " deg h=" << site.height.value() << " m\n";
    std::cout << "  x=" << std::fixed << std::setprecision(3)
              << ecef_m.x().value() << " m"
              << " (" << ecef_km.x().value() << " km)\n\n";

    const JulianDate jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});

    spherical::direction::ICRS vega_icrs(279.23473, 38.78369);
    auto vega_ecl   = vega_icrs.to_frame<frames::EclipticMeanJ2000>(jd);
    auto vega_true  = vega_icrs.to_frame<frames::EquatorialTrueOfDate>(jd);
    auto vega_horiz = vega_icrs.to_horizontal(jd, site);

    std::cout << "Direction transforms\n";
    std::cout << "  ICRS RA/Dec: " << vega_icrs.ra().value() << ", " << vega_icrs.dec().value() << " deg\n";
    std::cout << "  Ecliptic lon/lat: " << vega_ecl.lon().value() << ", " << vega_ecl.lat().value() << " deg\n";
    std::cout << "  True-of-date RA/Dec: " << vega_true.ra().value() << ", " << vega_true.dec().value() << " deg\n";
    std::cout << "  Horizontal az/alt: " << vega_horiz.az().value() << ", " << vega_horiz.alt().value() << " deg\n\n";

    spherical::position::ICRS<qtty::AstronomicalUnit> synthetic_star(
        qtty::Degree(210.0), qtty::Degree(-12.0), qtty::AstronomicalUnit(4.2));

    cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit> earth =
        ephemeris::earth_heliocentric(jd);

    std::cout << "Typed positions\n";
    std::cout << "  Synthetic star distance: " << synthetic_star.distance().value() << " AU\n";
    std::cout << "  Earth heliocentric x: " << earth.x().value() << " AU\n";

    return 0;
}
