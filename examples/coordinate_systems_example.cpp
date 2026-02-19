/**
 * @file coordinate_systems_example.cpp
 * @brief Coordinate systems and frame transform walkthrough.
 *
 * Usage:
 *   cmake --build build-make --target coordinate_systems_example
 *   ./build-make/coordinate_systems_example
 */

#include <siderust/siderust.hpp>

#include <cstdio>

using namespace siderust;
using namespace siderust::frames;

int main() {
    std::printf("=== Coordinate Systems Example ===\n\n");

    auto obs = roque_de_los_muchachos();
    auto jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});

    std::printf("Observer (Geodetic): lon=%.4f deg lat=%.4f deg h=%.1f m\n",
                obs.lon.value(), obs.lat.value(), obs.height.value());

    auto ecef_m = obs.to_cartesian<qtty::Meter>();
    auto ecef_km = obs.to_cartesian<qtty::Kilometer>();
    std::printf("Observer (ECEF): x=%.2f m y=%.2f m z=%.2f m\n",
                ecef_m.x().value(), ecef_m.y().value(), ecef_m.z().value());
    std::printf("Observer (ECEF): x=%.2f km y=%.2f km z=%.2f km\n\n",
                ecef_km.x().value(), ecef_km.y().value(), ecef_km.z().value());

    // Vega J2000 ICRS direction.
    spherical::direction::ICRS vega_icrs(279.23473, 38.78369);

    auto vega_ecl = vega_icrs.to<EclipticMeanJ2000>(jd);
    auto vega_eq_mod = vega_icrs.to<EquatorialMeanOfDate>(jd);
    auto vega_eq_tod = vega_icrs.to<EquatorialTrueOfDate>(jd);
    auto vega_hor = vega_icrs.to_horizontal(jd, obs);
    auto vega_back = vega_ecl.to<ICRS>(jd);

    std::printf("Vega ICRS: RA=%.6f Dec=%.6f\n",
                vega_icrs.ra().value(), vega_icrs.dec().value());
    std::printf("Vega EclipticMeanJ2000: lon=%.6f lat=%.6f\n",
                vega_ecl.lon().value(), vega_ecl.lat().value());
    std::printf("Vega EquatorialMeanOfDate: RA=%.6f Dec=%.6f\n",
                vega_eq_mod.ra().value(), vega_eq_mod.dec().value());
    std::printf("Vega EquatorialTrueOfDate: RA=%.6f Dec=%.6f\n",
                vega_eq_tod.ra().value(), vega_eq_tod.dec().value());
    std::printf("Vega Horizontal: az=%.6f alt=%.6f\n",
                vega_hor.az().value(), vega_hor.alt().value());
    std::printf("Vega roundtrip ICRS<-Ecliptic: RA=%.6f Dec=%.6f\n\n",
                vega_back.ra().value(), vega_back.dec().value());

    spherical::position::ICRS<qtty::AstronomicalUnit> target_sph_au(
        qtty::Degree(120.0), qtty::Degree(-25.0), qtty::AstronomicalUnit(2.0)
    );
    auto target_dir = target_sph_au.direction();
    std::printf("Spherical ICRS position: RA=%.2f Dec=%.2f dist=%.3f AU\n",
                target_sph_au.ra().value(),
                target_sph_au.dec().value(),
                target_sph_au.distance().value());
    std::printf("Direction extracted from spherical position: RA=%.2f Dec=%.2f\n\n",
                target_dir.ra().value(), target_dir.dec().value());

    cartesian::position::ICRS<qtty::Meter> target_cart_m(1.5e11, -3.0e10, 2.0e10);
    cartesian::position::ICRS<qtty::AstronomicalUnit> target_cart_au(
        target_cart_m.x().to<qtty::AstronomicalUnit>(),
        target_cart_m.y().to<qtty::AstronomicalUnit>(),
        target_cart_m.z().to<qtty::AstronomicalUnit>()
    );

    std::printf("Cartesian ICRS position: x=%.3e m y=%.3e m z=%.3e m\n",
                target_cart_m.x().value(),
                target_cart_m.y().value(),
                target_cart_m.z().value());
    std::printf("Cartesian ICRS position: x=%.6f AU y=%.6f AU z=%.6f AU\n",
                target_cart_au.x().value(),
                target_cart_au.y().value(),
                target_cart_au.z().value());

    return 0;
}
