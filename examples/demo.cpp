/**
 * @file demo.cpp
 * @brief Demonstrates the siderust C++ API.
 *
 * Usage:
 *   cd build && cmake .. && cmake --build . && ./demo
 */

#include <siderust/siderust.hpp>
#include <cmath>
#include <cstdio>

int main() {
    using namespace siderust;

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
                obs.lon_deg, obs.lat_deg, obs.height_m);

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

    // --- Coordinate transform ---
    SphericalDirection icrs(279.23473, 38.78369, Frame::ICRS);
    auto ecl = icrs.transform(Frame::EclipticMeanJ2000, jd.value());
    std::printf("ICRS (%.4f, %.4f) -> EclMeanJ2000 (%.4f, %.4f)\n\n",
                icrs.lon_deg, icrs.lat_deg, ecl.lon_deg, ecl.lat_deg);

    // --- Horizontal ---
    auto hor = icrs.to_horizontal(obs, jd.value());
    std::printf("Horizontal: az=%.4f  alt=%.4f deg\n\n",
                hor.azimuth_deg(), hor.altitude_deg());

    // --- Planets ---
    auto mars_data = mars();
    std::printf("Mars: mass=%.4e kg, radius=%.2f km\n",
                mars_data.mass_kg, mars_data.radius_km);
    std::printf("  orbit: a=%.6f AU, e=%.6f\n\n",
                mars_data.orbit.semi_major_axis_au,
                mars_data.orbit.eccentricity);

    // --- Ephemeris ---
    auto earth_pos = ephemeris::earth_heliocentric(jd);
    std::printf("Earth heliocentric: (%.8f, %.8f, %.8f) AU\n",
                earth_pos.x, earth_pos.y, earth_pos.z);

    auto moon_pos = ephemeris::moon_geocentric(jd);
    std::printf("Moon geocentric:    (%.2f, %.2f, %.2f) km\n\n",
                moon_pos.x, moon_pos.y, moon_pos.z);

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
