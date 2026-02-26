/**
 * @file 15_targets_proper_motion.cpp
 * @brief C++ port of siderust/examples/40_targets_proper_motion.rs
 *
 * Demonstrates proper motion propagation for catalog stars.  In Rust the
 * `CoordinateWithPM` type propagates RA/Dec/proper-motion from a reference
 * epoch.  This binding is **not yet available** in C++; a placeholder section
 * clearly marks that area, while the surrounding coordinate arithmetic is
 * fully implemented.
 *
 * Run with: cmake --build build --target targets_proper_motion_example
 */

#include <cmath>
#include <iomanip>
#include <iostream>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;
using namespace siderust::frames;

// TODO: [PLACEHOLDER] CoordinateWithPM / set_proper_motion_since_j2000 not
// yet bound in C++. The Rust API is:
//   let star_pm = star.set_proper_motion_since_j2000(target_jd);
// These will be added once the FFI layer for proper motion propagation is
// implemented.

int main() {
    std::cout << "=== Targets with Proper Motion ===\n\n";

    // -------------------------------------------------------------------------
    // Part 1: Catalog star directions (static, no proper motion — fully bound)
    // -------------------------------------------------------------------------
    std::cout << "--- Part 1: Catalog Stars — Current ICRS Directions ---\n\n";

    struct CatalogEntry {
        const char *name;
        double ra_deg;     // J2000 ICRS right ascension (degrees)
        double dec_deg;    // J2000 ICRS declination (degrees)
        double pmRA_mas;   // proper motion in RA (mas/yr)
        double pmDec_mas;  // proper motion in Dec (mas/yr)
    };

    // J2000 ICRS coords and proper motions (Hipparcos/Gaia)
    const CatalogEntry entries[] = {
        {"Sirius",     101.2871, -16.7161,  -546.0, -1223.0},
        {"Vega",       279.2348,  38.7837,  +200.9,  +287.5},
        {"Polaris",     37.9546,  89.2641,   +44.2,   -11.7},
        {"Altair",     297.6958,   8.8683,  +536.8,  +385.3},
        {"Arcturus",   213.9153,  19.1822, -1093.4, -1999.4},
        {"Betelgeuse",  88.7929,   7.4070,   +27.3,   +10.9},
        {"Rigel",       78.6345,  -8.2016,    +1.3,    -0.1},
        {"Aldebaran",   68.9801,  16.5093,   +63.5,  -189.9},
    };

    std::cout << std::setw(12) << "Star"
              << std::setw(12) << "RA (°)"
              << std::setw(12) << "Dec (°)"
              << std::setw(16) << "pmRA (mas/yr)"
              << std::setw(16) << "pmDec (mas/yr)"
              << "\n";
    std::cout << std::string(68, '-') << "\n";

    for (const auto &e : entries) {
        std::cout << std::left << std::setw(12) << e.name
                  << std::right << std::setw(12) << std::fixed << std::setprecision(4)
                  << e.ra_deg
                  << std::setw(12) << e.dec_deg
                  << std::setw(16) << std::setprecision(1) << e.pmRA_mas
                  << std::setw(16) << e.pmDec_mas << "\n";
    }

    // -------------------------------------------------------------------------
    // Part 2: Manual proper-motion propagation (simplified linear model)
    //         This is what the Rust CoordinateWithPM does internally.
    // -------------------------------------------------------------------------
    std::cout << "\n--- Part 2: Manual Linear Proper Motion Propagation ---\n";
    std::cout << "(Simplified — no parallax correction. Rust uses a full ICRS model.)\n\n";

    const double j2000_jd   = 2451545.0;
    const double target_jd  = 2451545.0 + 100.0 * 365.25; // J2100.0
    const double years       = (target_jd - j2000_jd) / 365.25;

    std::cout << "Propagating from J2000.0 to J2100.0 (" << std::setprecision(1)
              << years << " years)\n\n";

    for (const auto &e : entries) {
        const double ra0  = e.ra_deg;   // J2000 RA in degrees
        const double dec0 = e.dec_deg;  // J2000 Dec in degrees

        // Convert mas/yr → deg/yr
        const double pm_ra_deg  = e.pmRA_mas  / (3.6e6 * std::cos(dec0 * M_PI / 180.0));
        const double pm_dec_deg = e.pmDec_mas / 3.6e6;

        const double ra1  = ra0  + pm_ra_deg  * years;
        const double dec1 = dec0 + pm_dec_deg * years;

        // Angular offset in arcseconds
        const double dra  = (ra1 - ra0) * 3600.0;
        const double ddec = (dec1 - dec0) * 3600.0;
        const double separation_arcsec = std::sqrt(dra * dra + ddec * ddec);

        std::cout << std::left << std::setw(12) << e.name
                  << "  RA: " << std::right << std::setw(10) << std::setprecision(4)
                  << ra0 << " → " << std::setw(10) << ra1
                  << "°   shift=" << std::setprecision(2) << separation_arcsec << " arcsec\n";
    }

    // -------------------------------------------------------------------------
    // Part 3: PLACEHOLDER — CoordinateWithPM
    // -------------------------------------------------------------------------
    std::cout << "\n--- Part 3: [PLACEHOLDER] CoordinateWithPM API ---\n";
    std::cout << "NOTE: The following Rust capability is not yet bound in C++:\n";
    std::cout << "  // Rust:\n";
    std::cout << "  let star = siderust::catalog::SIRIUS;\n";
    std::cout << "  let jd_target = Julian::J2100;\n";
    std::cout << "  let pos_2100 = star.set_proper_motion_since_j2000(jd_target);\n";
    std::cout << "  // Returns CoordinateWithPM — ICRS direction with full\n";
    std::cout << "  // rigorous proper-motion propagation (including parallax).\n";
    std::cout << "\n  // C++ equivalent (future API):\n";
    std::cout << "  // auto pos_2100 = SIRIUS.propagate_proper_motion(JulianDate_J2100);\n";
    std::cout << "\nUsing manual linear propagation above as approximation.\n";

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
