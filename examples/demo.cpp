/**
 * @file demo.cpp
 * @brief Demonstrates the siderust C++ API — both typed and legacy.
 *
 * Usage:
 *   cd build && cmake .. && cmake --build . && ./demo
 */

#include <siderust/siderust.hpp>
#include <cmath>
#include <cstdio>

int main() {
    using namespace siderust;
    using namespace siderust::frames;

    std::printf("=== siderust-cpp demo ===\n\n");

    // --- Time ---
    auto jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
    std::printf("JD for 2026-07-15 22:00 UTC: %.6f\n", jd.value());
    std::printf("Julian centuries since J2000: %.10f\n", jd.julian_centuries());

    auto mjd = MJD::from_jd(jd);
    std::printf("MJD: %.6f\n\n", mjd.value());

    // --- Observatory ---
    auto obs = roque_de_los_muchachos();
    std::printf("Roque de los Muchachos: lon=%.4f  lat=%.4f  h=%.0f m\n\n",
                obs.lon_deg(), obs.lat_deg(), obs.height_m());

    // --- Sun altitude ---
    double sun_alt = sun::altitude_at(obs, mjd);
    std::printf("Sun altitude: %.4f rad (%.2f deg)\n\n",
                sun_alt, sun_alt * 180.0 / M_PI);

    // --- Star catalog ---
    auto vega = Star::catalog("VEGA");
    std::printf("Star: %s, d=%.2f ly, L=%.2f Lsun\n",
                vega.name().c_str(), vega.distance_ly(),
                vega.luminosity_solar());

    // --- Star altitude ---
    double star_alt = star_altitude::altitude_at(vega, obs, mjd);
    std::printf("Vega altitude: %.4f rad (%.2f deg)\n\n",
                star_alt, star_alt * 180.0 / M_PI);

    // =================================================================
    // NEW TYPED API
    // =================================================================
    std::printf("--- Typed Coordinate API ---\n\n");

    // Compile-time typed ICRS direction
    IcrsDir vega_icrs(279.23473, 38.78369);

    // Template-targeted transform: ICRS → EclipticMeanJ2000
    auto ecl = vega_icrs.to_frame<EclipticMeanJ2000>(jd);
    std::printf("Typed ICRS (%.4f, %.4f) -> EclMeanJ2000 (%.4f, %.4f)\n",
                vega_icrs.lon_deg(), vega_icrs.lat_deg(),
                ecl.lon_deg(), ecl.lat_deg());

    // Shorthand .to<>() syntax
    auto eq_j2000 = vega_icrs.to<EquatorialMeanJ2000>(jd);
    std::printf("Typed ICRS -> EquatorialJ2000 (%.4f, %.4f)\n",
                eq_j2000.lon_deg(), eq_j2000.lat_deg());

    // Horizontal transform
    auto hor = vega_icrs.to_horizontal(jd, obs);
    std::printf("Typed Horizontal: az=%.4f  alt=%.4f deg\n\n",
                hor.azimuth_deg(), hor.altitude_deg());

    // Roundtrip: ICRS → Ecliptic → ICRS
    auto back = ecl.to_frame<ICRS>(jd);
    std::printf("Roundtrip: (%.6f, %.6f) -> (%.6f, %.6f) -> (%.6f, %.6f)\n",
                vega_icrs.lon_deg(), vega_icrs.lat_deg(),
                ecl.lon_deg(), ecl.lat_deg(),
                back.lon_deg(), back.lat_deg());

    // qtty unit-safe angle conversion
    qtty::Radian ra_rad = vega_icrs.lon.to<qtty::Radian>();
    std::printf("Vega RA: %.6f deg = %.6f rad\n\n", vega_icrs.lon_deg(), ra_rad.value());

    // --- Typed Ephemeris ---
    std::printf("--- Typed Ephemeris ---\n\n");

    auto earth = ephemeris::earth_heliocentric_typed(jd);
    std::printf("Earth heliocentric (typed AU): (%.8f, %.8f, %.8f)\n",
                earth.x(), earth.y(), earth.z());

    // Unit conversion: AU → km
    qtty::Kilometer x_km = earth.comp_x.to<qtty::Kilometer>();
    qtty::Kilometer y_km = earth.comp_y.to<qtty::Kilometer>();
    std::printf("Earth heliocentric (km): (%.2f, %.2f, ...)\n\n",
                x_km.value(), y_km.value());

    auto moon = ephemeris::moon_geocentric_typed(jd);
    std::printf("Moon geocentric (typed km): (%.2f, %.2f, %.2f)\n",
                moon.x(), moon.y(), moon.z());
    double moon_r = std::sqrt(moon.x()*moon.x() + moon.y()*moon.y() + moon.z()*moon.z());
    std::printf("Moon distance: %.2f km\n\n", moon_r);

    // =================================================================
    // LEGACY API (still works)
    // =================================================================
    std::printf("--- Legacy API ---\n\n");

    SphericalDirection icrs_legacy(279.23473, 38.78369, Frame::ICRS);
    auto ecl_legacy = icrs_legacy.transform(Frame::EclipticMeanJ2000, jd.value());
    std::printf("Legacy ICRS (%.4f, %.4f) -> EclMeanJ2000 (%.4f, %.4f)\n",
                icrs_legacy.lon_deg, icrs_legacy.lat_deg,
                ecl_legacy.lon_deg, ecl_legacy.lat_deg);

    auto hor_legacy = icrs_legacy.to_horizontal(obs, jd.value());
    std::printf("Legacy Horizontal: az=%.4f  alt=%.4f deg\n\n",
                hor_legacy.azimuth_deg(), hor_legacy.altitude_deg());

    // --- Planets ---
    auto mars_data = mars();
    std::printf("Mars: mass=%.4e kg, radius=%.2f km\n",
                mars_data.mass_kg, mars_data.radius_km);
    std::printf("  orbit: a=%.6f AU, e=%.6f\n\n",
                mars_data.orbit.semi_major_axis_au,
                mars_data.orbit.eccentricity);

    // --- Night periods (sun below -18°) ---
    auto night_start = mjd;
    auto night_end   = mjd + 1.0;
    auto nights = sun::below_threshold(obs, night_start, night_end, -18.0);
    std::printf("Astronomical night periods (sun < -18 deg):\n");
    for (auto& p : nights) {
        std::printf("  MJD %.6f – %.6f  (%.2f hours)\n",
                    p.start_mjd(), p.end_mjd(),
                    p.duration_days() * 24.0);
    }

    std::printf("\nDone.\n");
    return 0;
}
