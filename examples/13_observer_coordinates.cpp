/**
 * @file 13_observer_coordinates.cpp
 * @brief C++ port of siderust/examples/36_observer_coordinates.rs
 *
 * Shows how an observer's ground coordinates (geodetic) relate to
 * geocentric Cartesian coordinates, and how to convert between
 * equatorial and horizontal systems for an observation site.
 *
 * Run with: cmake --build build --target observer_coordinates_example
 */

#include <cmath>
#include <iomanip>
#include <iostream>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

int main() {
    std::cout << "=== Observer Coordinate Systems ===\n\n";

    // -------------------------------------------------------------------------
    // Build a few observatories using the Geodetic() factory
    // -------------------------------------------------------------------------
    const struct { const char *name; double lon; double lat; double alt_m; } sites[] = {
        {"Greenwich",            0.0,      51.4769,  46.0},
        {"Roque de los Muchachos", -17.892, 28.756, 2396.0},
        {"Mauna Kea",          -155.472,  19.826,  4207.0},
        {"El Paranal",          -70.403, -24.627,  2635.0},
        {"La Silla",            -70.730, -29.257,  2400.0},
    };

    std::cout << "--- Observatory Summary ---\n";
    std::cout << std::setw(28) << "Name"
              << std::setw(10) << "Lon(°)"
              << std::setw(10) << "Lat(°)"
              << std::setw(10) << "Alt(m)\n";
    std::cout << std::string(58, '-') << "\n";

    for (const auto &s : sites) {
        std::cout << std::setw(28) << std::left  << s.name
                  << std::setw(10) << std::right << std::fixed << std::setprecision(3)
                  << s.lon
                  << std::setw(10) << s.lat
                  << std::setw(10) << std::setprecision(0) << s.alt_m << "\n";
    }
    std::cout << "\n";

    // -------------------------------------------------------------------------
    // Use built-in observatory constants (pre-defined in siderust.hpp)
    // -------------------------------------------------------------------------
    std::cout << "--- Built-in Observatory Constants ---\n";
    const Geodetic obs1 = ROQUE_DE_LOS_MUCHACHOS;
    const Geodetic obs2 = MAUNA_KEA;
    const Geodetic obs3 = EL_PARANAL;
    const Geodetic obs4 = LA_SILLA_OBSERVATORY;

    std::cout << "ROQUE_DE_LOS_MUCHACHOS  lat=" << std::setprecision(3)
              << obs1.lat.value()  << "°  lon=" << obs1.lon.value() << "°"
              << "  alt=" << obs1.height.value() << " m\n";
    std::cout << "MAUNA_KEA               lat=" << obs2.lat.value()
              << "°  lon=" << obs2.lon.value() << "°"
              << "  alt=" << obs2.height.value() << " m\n";
    std::cout << "EL_PARANAL              lat=" << obs3.lat.value()
              << "°  lon=" << obs3.lon.value() << "°"
              << "  alt=" << obs3.height.value() << " m\n";
    std::cout << "LA_SILLA_OBSERVATORY    lat=" << obs4.lat.value()
              << "°  lon=" << obs4.lon.value() << "°"
              << "  alt=" << obs4.height.value() << " m\n\n";

    // -------------------------------------------------------------------------
    // Frame conversion: ICRS position → Horizontal for selected stars
    // -------------------------------------------------------------------------
    std::cout << "--- Star Horizontal Coordinates at Roque de los Muchachos ---\n";
    std::cout << "(Epoch: J2000.0, for rough indicative values)\n\n";

    const JulianDate jd(2451545.0); // J2000.0
    const Geodetic observer = ROQUE_DE_LOS_MUCHACHOS;

    // Known J2000 ICRS coordinates for catalog stars
    struct StarInfo { const char *name; double ra_deg; double dec_deg; };
    const StarInfo stars[] = {
        {"Sirius",     101.2871,  -16.7161},
        {"Vega",       279.2348,   38.7837},
        {"Altair",     297.6958,    8.8683},
        {"Polaris",     37.9546,   89.2641},
        {"Betelgeuse",  88.7929,    7.4070},
    };

    std::cout << std::setw(12) << "Star"
              << std::setw(12) << " RA (°)"
              << std::setw(12) << "Dec (°)"
              << std::setw(12) << "Alt (°)\n";
    std::cout << std::string(48, '-') << "\n";

    for (const auto &s : stars) {
        const spherical::direction::ICRS dir(qtty::Degree(s.ra_deg), qtty::Degree(s.dec_deg));
        const auto h_dir = dir.to_horizontal(jd, observer);
        std::cout << std::left  << std::setw(12) << s.name
                  << std::right << std::setw(12) << std::setprecision(3) << std::fixed
                  << s.ra_deg
                  << std::setw(12) << s.dec_deg
                  << std::setw(12) << h_dir.altitude().value() << "\n";
    }

    // -------------------------------------------------------------------------
    // Topocentric vs geocentric: a brief note and illustration
    // -------------------------------------------------------------------------
    std::cout << "\n--- Topocentric Parallax (Moon) ---\n";
    const auto moon_geo = ephemeris::moon_geocentric(jd);
    {
        const auto moon_sph = moon_geo.to_spherical();
        std::cout << "Moon geocentric:   r=" << std::setprecision(6)
                  << moon_sph.distance().value() << " AU\n";
    }
    // Topocentric shift is computed internally by the altitude/frame routines;
    // this example illustrates that raw ephemeris gives geocentric positions.
    std::cout << "(Altitude routines apply topocentric correction automatically)\n";

    // -------------------------------------------------------------------------
    // Summary: compare observers at different latitudes
    // -------------------------------------------------------------------------
    std::cout << "\n--- Sirius Visibility by Latitude ---\n";
    std::cout << "  (hours above horizon over one year via altitude periods API)\n\n";

    const Period full_year(MJD(60000.0), MJD(60365.0));
    struct LatSite { const char *name; Geodetic obs; };
    const LatSite lat_sites[] = {
        {"Greenwich (+51.5°)", Geodetic(0.0,  51.48, 0.0)},
        {"Roque  (+28.8°)",    ROQUE_DE_LOS_MUCHACHOS},
        {"Paranal (-24.6°)",   EL_PARANAL},
    };

    for (const auto &ls : lat_sites) {
        const auto periods = star_altitude::above_threshold(SIRIUS, ls.obs, full_year,
                                                             qtty::Degree(0.0));
        double total_h = 0.0;
        for (const auto &p : periods)
            total_h += p.duration<qtty::Hour>().value();
        std::cout << "  " << std::left << std::setw(22) << ls.name
                  << "Sirius above horizon: " << std::setprecision(0) << total_h << " h/yr\n";
    }

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
