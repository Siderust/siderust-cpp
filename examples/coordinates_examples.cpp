/**
 * @file coordinates_examples.cpp
 * @brief Focused examples for creating and converting typed coordinates.
 *
 * Usage:
 *   cmake --build build-make --target coordinates_examples
 *   ./build-make/coordinates_examples
 */

#include <siderust/siderust.hpp>

#include <cstdio>

using namespace siderust;

static void geodetic_and_ecef_example() {
    std::printf("1) Geodetic -> ECEF cartesian\n");

    Geodetic obs(-17.8890, 28.7610, 2396.0);
    auto ecef = obs.to_cartesian();
    auto ecef_km = obs.to_cartesian<qtty::Kilometer>();

    std::printf("   Geodetic lon=%.4f deg lat=%.4f deg h=%.1f m\n",
                obs.lon.value(), obs.lat.value(), obs.height.value());
    std::printf("   ECEF x=%.2f m y=%.2f m z=%.2f m\n\n",
                ecef.x().value(), ecef.y().value(), ecef.z().value());
    std::printf("   ECEF x=%.2f km y=%.2f km z=%.2f km\n\n",
                ecef_km.x().value(), ecef_km.y().value(), ecef_km.z().value());
}

static void spherical_direction_example() {
    std::printf("2) Spherical direction frame conversions\n");

    spherical::direction::ICRS vega_icrs(279.23473, 38.78369);
    auto jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});

    auto ecl = vega_icrs.to<frames::EclipticMeanJ2000>(jd);
    auto eq_mod = vega_icrs.to<frames::EquatorialMeanOfDate>(jd);
    auto hor = vega_icrs.to_horizontal(jd, roque_de_los_muchachos());

    std::printf("   ICRS lon=%.5f lat=%.5f\n", vega_icrs.lon.value(), vega_icrs.lat.value());
    std::printf("   Ecliptic lon=%.5f lat=%.5f\n", ecl.lon.value(), ecl.lat.value());
    std::printf("   Equatorial(MOD) RA=%.5f Dec=%.5f\n", eq_mod.ra().value(), eq_mod.dec().value());
    std::printf("   Horizontal az=%.5f alt=%.5f\n\n", hor.az().value(), hor.alt().value());
}

static void spherical_position_example() {
    std::printf("3) Spherical position + extracting direction\n");

    spherical::position::ICRS<qtty::Meter> target(
        qtty::Degree(120.0), qtty::Degree(-25.0), qtty::Meter(2.0e17)
    );
    auto dir = target.direction();

    std::printf("   Position lon=%.2f lat=%.2f dist=%.3e m\n",
                target.lon.value(), target.lat.value(), target.distance().value());
    std::printf("   Direction-only lon=%.2f lat=%.2f\n\n",
                dir.lon.value(), dir.lat.value());
}

static void cartesian_and_units_example() {
    std::printf("4) Cartesian coordinate creation + unit conversion\n");

    cartesian::Direction<frames::ICRS> axis_x(1.0, 0.0, 0.0);
    cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit> sample_helio_au(1.0, 0.25, -0.1);

    auto x_km = sample_helio_au.x().to<qtty::Kilometer>();
    auto y_km = sample_helio_au.y().to<qtty::Kilometer>();

    std::printf("   Direction<ICRS> x=%.1f y=%.1f z=%.1f\n", axis_x.x, axis_x.y, axis_x.z);
    std::printf("   Position<Heliocentric,Ecl,AU> x=%.3f AU y=%.3f AU z=%.3f AU\n",
                sample_helio_au.x().value(), sample_helio_au.y().value(), sample_helio_au.z().value());
    std::printf("   Same position in km x=%.2f y=%.2f\n\n", x_km.value(), y_km.value());
}

static void ephemeris_typed_example() {
    std::printf("5) Typed ephemeris coordinates\n");

    auto jd = JulianDate::J2000();
    auto earth = ephemeris::earth_heliocentric(jd);  // cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>
    auto moon = ephemeris::moon_geocentric(jd);      // cartesian::position::MoonGeocentric<qtty::Kilometer>

    std::printf("   Earth heliocentric (AU) x=%.8f y=%.8f z=%.8f\n",
                earth.x().value(), earth.y().value(), earth.z().value());
    std::printf("   Moon geocentric (km) x=%.3f y=%.3f z=%.3f\n\n",
                moon.x().value(), moon.y().value(), moon.z().value());
}

int main() {
    std::printf("=== Coordinate Creation & Conversion Examples ===\n\n");

    geodetic_and_ecef_example();
    spherical_direction_example();
    spherical_position_example();
    cartesian_and_units_example();
    ephemeris_typed_example();

    std::printf("Done.\n");
    return 0;
}
