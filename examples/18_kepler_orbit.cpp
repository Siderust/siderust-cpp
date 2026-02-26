/**
 * @file 18_kepler_orbit.cpp
 * @brief C++ port of siderust/examples/33_kepler_orbit.rs
 *
 * Demonstrates Keplerian orbit propagation.  The `Orbit` struct is fully
 * available in C++; the `kepler_position()` / `solve_keplers_equation()`
 * free functions are **not yet bound** (placeholder section below).
 *
 * Run with: cmake --build build --target kepler_orbit_example
 */

#include <cmath>
#include <iomanip>
#include <iostream>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;
using namespace siderust::frames;

// TODO: [PLACEHOLDER] solve_keplers_equation() and kepler_position() not yet
// exposed in the C++ FFI wrapper. The Rust API is:
//   let pos = kepler_position(&orbit, jd);
// The `BodycentricParams` constructor uses them internally via to_bodycentric().

// Simple manual Kepler solver for demonstration (borrowed from celestial mechanics)
namespace demo {

/// Solve Kepler's equation M = E - e*sin(E) via Newton-Raphson.
static double solve_kepler(double M_rad, double e, int max_iter = 100) {
    double E = M_rad; // initial guess
    for (int i = 0; i < max_iter; ++i) {
        const double dE = (M_rad - E + e * std::sin(E)) / (1.0 - e * std::cos(E));
        E += dE;
        if (std::abs(dE) < 1e-12) break;
    }
    return E;
}

/// Propagate two-body orbit: returns heliocentric (x, y, 0) in AU at epoch+dt_days.
static std::pair<double, double>
propagate_orbit(const Orbit &orb, double dt_days) {
    // Mean motion (rad/day) = 2π / T, T = a^1.5 years * 365.25
    const double T_days = std::pow(orb.semi_major_axis_au, 1.5) * 365.25;
    const double n      = 2.0 * M_PI / T_days;

    const double M = orb.mean_anomaly_deg * M_PI / 180.0 + n * dt_days;
    const double E = solve_kepler(std::fmod(M, 2.0 * M_PI), orb.eccentricity);

    const double nu = 2.0 * std::atan2(std::sqrt(1.0 + orb.eccentricity) * std::sin(E / 2.0),
                                        std::sqrt(1.0 - orb.eccentricity) * std::cos(E / 2.0));

    const double r  = orb.semi_major_axis_au * (1.0 - orb.eccentricity * std::cos(E));
    const double om = orb.arg_perihelion_deg * M_PI / 180.0;
    const double x  = r * std::cos(nu + om);
    const double y  = r * std::sin(nu + om);
    return {x, y};
}

} // namespace demo

int main() {
    std::cout << "=== Kepler Orbit Propagation ===\n\n";

    // -------------------------------------------------------------------------
    // PLACEHOLDER note
    // -------------------------------------------------------------------------
    std::cout << "[PLACEHOLDER] Rust's kepler_position() is not yet bound in C++.\n";
    std::cout << "Using a manual Newton-Raphson Kepler solver for demonstration.\n\n";

    // -------------------------------------------------------------------------
    // Earth's orbit elements (J2000.0)
    // -------------------------------------------------------------------------
    const JulianDate jd0 = JulianDate::J2000();

    const Orbit earth_orbit{
        1.0000010178,  // semi-major axis (AU)
        0.0167086,     // eccentricity
        0.0000001,     // inclination (°)
        0.0,           // RAAN (°)
        102.9373481,   // argument of periapsis (°)
        100.4645717,   // mean anomaly at epoch (°)
        jd0.value()    // epoch JD
    };

    // -------------------------------------------------------------------------
    // Part 1: Display orbital elements
    // -------------------------------------------------------------------------
    std::cout << "--- Earth's Keplerian Elements (J2000.0) ---\n";
    std::cout << "  semi-major axis a = " << std::setprecision(7)
              << earth_orbit.semi_major_axis_au << " AU\n";
    std::cout << "  eccentricity    e = " << earth_orbit.eccentricity << "\n";
    std::cout << "  inclination     i = " << earth_orbit.inclination_deg << "°\n";
    std::cout << "  arg periapsis ω = " << earth_orbit.arg_perihelion_deg << "°\n";
    std::cout << "  mean anomaly  M₀ = " << earth_orbit.mean_anomaly_deg << "°\n";
    std::cout << "  epoch           = JD " << std::setprecision(1) << earth_orbit.epoch_jd << "\n\n";

    // -------------------------------------------------------------------------
    // Part 2: Propagate and compare with VSOP87
    // -------------------------------------------------------------------------
    std::cout << "--- Propagated Position vs VSOP87 (first 5 years) ---\n\n";
    std::cout << std::setw(10) << "Days" << std::setw(12) << "x_kepl(AU)"
              << std::setw(12) << "y_kepl(AU)" << std::setw(12) << "r_kepl(AU)"
              << std::setw(12) << "r_VSOP(AU)" << "\n";
    std::cout << std::string(58, '-') << "\n";

    for (int d : {0, 91, 182, 273, 365, 730, 1461}) {
        const auto [xk, yk] = demo::propagate_orbit(earth_orbit, d);
        const double rk     = std::sqrt(xk * xk + yk * yk);

        const JulianDate jd_t(jd0.value() + d);
        const auto vsop = ephemeris::earth_heliocentric(jd_t);
        const double rv = vsop.to_spherical().distance().value();

        std::cout << std::setw(10) << d
                  << std::setw(12) << std::setprecision(6) << std::fixed << xk
                  << std::setw(12) << yk
                  << std::setw(12) << rk
                  << std::setw(12) << rv << "\n";
    }

    // -------------------------------------------------------------------------
    // Part 3: Mars orbit propagation
    // -------------------------------------------------------------------------
    std::cout << "\n--- Mars Orbit (2-year trace, Kepler propagator) ---\n";

    const Orbit mars_orbit{
        1.523679342,   // a (AU)
        0.0934005,     // e
        1.849691,      // i (°)
        49.4785,       // RAAN (°)
        286.502,       // ω (°)
        19.37215,      // M₀ (°)
        jd0.value()
    };

    for (int d : {0, 182, 365, 548, 730}) {
        const auto [xm, ym] = demo::propagate_orbit(mars_orbit, d);
        const double rm     = std::sqrt(xm * xm + ym * ym);
        const double lon_m  = std::atan2(ym, xm) * 180.0 / M_PI;
        if (lon_m < 0) {}  // suppress unused warning
        std::cout << "  day=" << std::setw(4) << d
                  << "  x=" << std::setw(10) << std::setprecision(5) << xm
                  << "  y=" << std::setw(10) << ym
                  << "  r=" << std::setprecision(4) << rm << " AU\n";
    }

    // -------------------------------------------------------------------------
    // Part 4: to_bodycentric round-trip (uses kepler internally)
    // -------------------------------------------------------------------------
    std::cout << "\n--- to_bodycentric() uses Kepler solver internally ---\n";
    using namespace siderust::centers;
    const BodycentricParams mars_params = BodycentricParams::heliocentric(mars_orbit);
    const auto mars_hel = ephemeris::mars_heliocentric(jd0);

    const auto mars_bc_rel = to_bodycentric(mars_hel, mars_params, jd0);
    std::cout << "Mars relative to itself (should be ~0):\n";
    std::cout << "  x=" << std::scientific << std::setprecision(2)
              << mars_bc_rel.pos.x().value()
              << "  y=" << mars_bc_rel.pos.y().value()
              << "  z=" << mars_bc_rel.pos.z().value() << "\n";

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
