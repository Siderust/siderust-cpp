/**
 * @file 16_jpl_precise_ephemeris.cpp
 * @brief C++ port of siderust/examples/32_jpl_precise_ephemeris.rs
 *
 * [PLACEHOLDER] — JPL DE430/DE440 high-precision ephemeris is **not yet
 * bound** in the C++ wrapper. The Rust implementation uses a run-time
 * loadable ephemeris file; the C++ FFI layer currently only exposes the
 * VSOP87 analytical series.
 *
 * This file documents what the API will look like and shows the VSOP87
 * baseline for comparison.
 *
 * Run with: cmake --build build --target jpl_precise_ephemeris_example
 */

#include <iomanip>
#include <iostream>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

int main() {
    std::cout << "=== JPL Precise Ephemeris  [PLACEHOLDER] ===\n\n";

    // -------------------------------------------------------------------------
    // PLACEHOLDER: JPL DE440 ephemeris loading
    // -------------------------------------------------------------------------
    std::cout << "NOTE: JPL DE440/DE441 ephemeris not yet available in C++ bindings.\n";
    std::cout << "  // Rust:\n";
    std::cout << "  // use siderust::ephemeris::jpl::*;\n";
    std::cout << "  // let ephem = JplEphemeris::load(\"de440.bsp\");\n";
    std::cout << "  // let pos = ephem.mars_heliocentric(jd);\n";
    std::cout << "\n  // C++ (future API):\n";
    std::cout << "  // auto ephem = jpl::load(\"de440.bsp\");\n";
    std::cout << "  // auto pos   = ephem.mars_heliocentric(jd);\n\n";

    // -------------------------------------------------------------------------
    // Fallback: VSOP87 comparison (fully available today)
    // -------------------------------------------------------------------------
    std::cout << "--- Baseline: VSOP87 Analytical Ephemeris (available now) ---\n\n";

    const JulianDate jd = JulianDate::J2000();
    std::cout << "Epoch: J2000.0 (JD " << std::fixed << std::setprecision(1)
              << jd.value() << ")\n\n";

    const auto earth_h = ephemeris::earth_heliocentric(jd);
    const auto mars_h  = ephemeris::mars_heliocentric(jd);
    const auto venus_h = ephemeris::venus_heliocentric(jd);
    const auto moon_g  = ephemeris::moon_geocentric(jd);

    auto print_body = [](const char *name, auto pos) {
        const auto sph = pos.to_spherical();
        std::cout << std::left << std::setw(8) << name
                  << "  r=" << std::right << std::setprecision(6)
                  << std::fixed << sph.distance().value() << " AU"
                  << "  lon=" << std::setw(10) << std::setprecision(4)
                  << sph.direction().longitude().value() << "°"
                  << "  lat=" << sph.direction().latitude().value() << "°\n";
    };

    std::cout << "Heliocentric positions (VSOP87):\n";
    print_body("Earth",  earth_h);
    print_body("Venus",  venus_h);
    print_body("Mars",   mars_h);

    std::cout << "\nGeocentric Moon (VSOP87-based):\n";
    {
        const auto sph = moon_g.to_spherical();
        std::cout << "  Moon  r=" << std::setprecision(6) << sph.distance().value()
                  << " AU  lon=" << std::setprecision(4) << sph.direction().longitude().value()
                  << "°  lat=" << sph.direction().latitude().value() << "°\n";
    }

    // -------------------------------------------------------------------------
    // Expected accuracy note (once JPL is available)
    // -------------------------------------------------------------------------
    std::cout << "\n--- Expected accuracy improvement with DE440 ---\n";
    std::cout << "  VSOP87 typical error:      ~1 arcsecond (inner planets)\n";
    std::cout << "  DE440 typical error:       ~0.001 arcsecond\n";
    std::cout << "  (Factor ~1000 improvement, relevant for high-precision astrometry)\n";

    std::cout << "\nImplementation status: VSOP87 fully available, DE440 — TODO.\n";
    std::cout << "\n=== Example Complete (Placeholder) ===\n";
    return 0;
}
