/**
 * @file solar_system_bodies_example.cpp
 * @brief Solar-system body ephemeris and catalog examples.
 *
 * Usage:
 *   cmake --build build-make --target solar_system_bodies_example
 *   ./build-make/solar_system_bodies_example
 */

#include <siderust/siderust.hpp>

#include <cmath>
#include <cstdio>

using namespace siderust;

template <typename PosT>
static double norm3(const PosT& p) {
    const double x = p.x().value();
    const double y = p.y().value();
    const double z = p.z().value();
    return std::sqrt(x * x + y * y + z * z);
}

static void print_planet(const char* name, const Planet& p) {
    std::printf("%-8s mass=%.4e kg  radius=%.1f km  a=%.6f AU  e=%.6f  i=%.3f deg\n",
                name,
                p.mass_kg,
                p.radius_km,
                p.orbit.semi_major_axis_au,
                p.orbit.eccentricity,
                p.orbit.inclination_deg);
}

int main() {
    std::printf("=== Solar System Bodies Example ===\n\n");

    auto jd = JulianDate::from_utc({2026, 7, 15, 0, 0, 0});
    std::printf("Epoch JD: %.6f\n\n", jd.value());

    auto sun_bary    = ephemeris::sun_barycentric(jd);
    auto earth_bary  = ephemeris::earth_barycentric(jd);
    auto earth_helio = ephemeris::earth_heliocentric(jd);
    auto moon_geo    = ephemeris::moon_geocentric(jd);

    std::printf("Sun barycentric (EclipticMeanJ2000, AU):\n");
    std::printf("  x=%.9f y=%.9f z=%.9f\n",
                sun_bary.x().value(), sun_bary.y().value(), sun_bary.z().value());
    std::printf("Earth barycentric (EclipticMeanJ2000, AU):\n");
    std::printf("  x=%.9f y=%.9f z=%.9f\n",
                earth_bary.x().value(), earth_bary.y().value(), earth_bary.z().value());
    std::printf("Earth heliocentric (EclipticMeanJ2000, AU):\n");
    std::printf("  x=%.9f y=%.9f z=%.9f\n",
                earth_helio.x().value(), earth_helio.y().value(), earth_helio.z().value());
    std::printf("Moon geocentric (EclipticMeanJ2000, km):\n");
    std::printf("  x=%.3f y=%.3f z=%.3f\n\n",
                moon_geo.x().value(), moon_geo.y().value(), moon_geo.z().value());

    const double earth_sun_au = norm3(earth_helio);
    const double moon_dist_km = norm3(moon_geo);
    std::printf("Earth-Sun distance: %.6f AU\n", earth_sun_au);
    std::printf("Moon distance from geocenter: %.2f km\n", moon_dist_km);

    const qtty::Kilometer earth_x_km = earth_helio.x().to<qtty::Kilometer>();
    std::printf("Earth heliocentric x component: %.2f km\n\n", earth_x_km.value());

    std::printf("Planet catalog (static properties):\n");
    print_planet("Mercury", MERCURY);
    print_planet("Venus", VENUS);
    print_planet("Earth", EARTH);
    print_planet("Mars", MARS);
    print_planet("Jupiter", JUPITER);
    print_planet("Saturn", SATURN);
    print_planet("Uranus", URANUS);
    print_planet("Neptune", NEPTUNE);

    return 0;
}
