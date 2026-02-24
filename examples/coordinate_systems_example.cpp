/**
 * @file coordinate_systems_example.cpp
 * @example coordinate_systems_example.cpp
 * @brief Compile-time frame tags and transform capabilities walkthrough.
 */

#include <iomanip>
#include <iostream>
#include <type_traits>

#include <siderust/siderust.hpp>

int main() {
    using namespace siderust;
    using namespace siderust::frames;

    std::cout << "=== coordinate_systems_example ===\n";

    static_assert(has_frame_transform_v<ICRS, EclipticMeanJ2000>);
    static_assert(has_frame_transform_v<EclipticMeanJ2000, EquatorialTrueOfDate>);
    static_assert(has_horizontal_transform_v<ICRS>);

    const Geodetic observer = ROQUE_DE_LOS_MUCHACHOS;
    const JulianDate jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});

    spherical::Direction<ICRS> src(279.23473, 38.78369);
    const auto ecl  = src.to_frame<EclipticMeanJ2000>(jd);
    const auto mod  = src.to_frame<EquatorialMeanOfDate>(jd);
    const auto tod  = mod.to_frame<EquatorialTrueOfDate>(jd);
    const auto horiz = src.to_horizontal(jd, observer);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Observer\n";
    std::cout << "  lon=" << observer.lon.value() << " deg"
              << " lat=" << observer.lat.value() << " deg\n\n";

    std::cout << "Frame transforms for Vega-like direction\n";
    std::cout << "  ICRS RA/Dec               : "
              << src.ra().value() << ", " << src.dec().value() << " deg\n";
    std::cout << "  EclipticMeanJ2000 lon/lat : "
              << ecl.lon().value() << ", " << ecl.lat().value() << " deg\n";
    std::cout << "  EquatorialMeanOfDate RA/Dec: "
              << mod.ra().value() << ", " << mod.dec().value() << " deg\n";
    std::cout << "  EquatorialTrueOfDate RA/Dec: "
              << tod.ra().value() << ", " << tod.dec().value() << " deg\n";
    std::cout << "  Horizontal az/alt         : "
              << horiz.az().value() << ", " << horiz.alt().value() << " deg\n\n";

    const auto ecef = observer.to_cartesian<qtty::Kilometer>();
    std::cout << "Observer in ECEF\n";
    std::cout << "  x=" << ecef.x().value() << " km"
              << " y=" << ecef.y().value() << " km"
              << " z=" << ecef.z().value() << " km\n";

    return 0;
}
