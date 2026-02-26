/**
 * @file 19_brent_root_finding.cpp
 * @brief C++ port of siderust/examples/28_brent_root_finding.rs
 *
 * [PLACEHOLDER] — The siderust Brent root-finding utility (`brent_find_root`)
 * is **not yet exposed** in the C++ FFI wrapper.  The surrounding altitude
 * search infrastructure (used internally by `above_threshold` et al.) is
 * fully available.
 *
 * This file demonstrates the concept with a simple manual Brent implementation
 * and documents what the Rust API looks like.
 *
 * Run with: cmake --build build --target brent_root_finding_example
 */

#include <cassert>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

// TODO: [PLACEHOLDER] siderust::brent_find_root() not yet bound in C++.
// Rust API:
//   use siderust::math::brent_find_root;
//   let root = brent_find_root(a, b, tol, |x| f(x));

// ---------------------------------------------------------------------------
// Manual Brent's method (ISO 9222 variant, fully adequate as placeholder)
// ---------------------------------------------------------------------------
static double brent_find_root(double a, double b, double tol,
                               const std::function<double(double)> &f) {
    double fa = f(a), fb = f(b);
    if (fa * fb > 0.0)
        throw std::runtime_error("brent: f(a) and f(b) must have opposite signs");

    double c = a, fc = fa, s, d = 0.0;
    bool mflag = true;

    for (int iter = 0; iter < 200; ++iter) {
        if (std::abs(b - a) < tol) break;

        if (fa != fc && fb != fc) {
            // Inverse quadratic interpolation
            s = a * fb * fc / ((fa - fb) * (fa - fc)) +
                b * fa * fc / ((fb - fa) * (fb - fc)) +
                c * fa * fb / ((fc - fa) * (fc - fb));
        } else {
            // Secant method
            s = b - fb * (b - a) / (fb - fa);
        }

        const double delta = std::abs(2.0 * tol * std::abs(b));
        const bool cond1   = (s < (3.0 * a + b) / 4.0 || s > b);
        const bool cond2   = (mflag && std::abs(s - b) >= std::abs(b - c) / 2.0);
        const bool cond3   = (!mflag && std::abs(s - b) >= std::abs(c - d) / 2.0);
        const bool cond4   = (mflag && std::abs(b - c) < delta);
        const bool cond5   = (!mflag && std::abs(c - d) < delta);

        if (cond1 || cond2 || cond3 || cond4 || cond5) {
            s = (a + b) / 2.0;
            mflag = true;
        } else {
            mflag = false;
        }

        double fs = f(s);
        d = c;
        c = b;
        fc = fb;

        if (fa * fs < 0.0) { b = s; fb = fs; }
        else               { a = s; fa = fs; }

        if (std::abs(fa) < std::abs(fb)) {
            std::swap(a, b); std::swap(fa, fb);
        }
    }
    return b;
}

int main() {
    std::cout << "=== Brent Root Finding  [PLACEHOLDER for native siderust API] ===\n\n";

    // -------------------------------------------------------------------------
    // PLACEHOLDER: Rust Brent API
    // -------------------------------------------------------------------------
    std::cout << "NOTE: siderust::math::brent_find_root() not yet bound in C++.\n";
    std::cout << "  // Rust:\n";
    std::cout << "  // let root = brent_find_root(0.0, 3.0, 1e-10, |x| x.sin() - 0.5);\n";
    std::cout << "  // // → π/6 ≈ 0.5235988...\n";
    std::cout << "\n  // C++ (future API):\n";
    std::cout << "  // double root = siderust::math::brent(0.0, 3.0, 1e-10,\n";
    std::cout << "  //     [](double x){ return std::sin(x) - 0.5; });\n\n";

    // -------------------------------------------------------------------------
    // Example 1: Simple trigonometric equation
    // -------------------------------------------------------------------------
    std::cout << "--- Example 1: sin(x) = 0.5  on [0, π/2] ---\n";
    const double root1 = brent_find_root(0.0, M_PI / 2.0, 1e-12,
                                          [](double x) { return std::sin(x) - 0.5; });
    std::cout << "  root        = " << std::setprecision(12) << root1 << "\n";
    std::cout << "  π/6         = " << M_PI / 6.0 << "\n";
    std::cout << "  |error|     = " << std::scientific << std::abs(root1 - M_PI / 6.0) << "\n\n";

    // -------------------------------------------------------------------------
    // Example 2: Kepler's equation  E - e*sin(E) = M
    // -------------------------------------------------------------------------
    std::cout << "--- Example 2: Kepler's equation M = E - e·sin(E) ---\n";
    const double e = 0.0934005; // Mars eccentricity
    const double M = 1.0;       // mean anomaly (rad)

    const double E = brent_find_root(0.0, 2.0 * M_PI, 1e-12,
                                      [e, M](double E) { return E - e * std::sin(E) - M; });

    std::cout << "  e=" << e << ", M=" << M << " rad\n";
    std::cout << "  Eccentric anomaly E = " << std::setprecision(10) << E << " rad\n";
    std::cout << "  Check: E - e·sin(E) = "
              << E - e * std::sin(E) << " (≈M=" << M << ")\n\n";

    // -------------------------------------------------------------------------
    // Example 3: Find Sun's altitude crossing time (siderust use-case)
    // -------------------------------------------------------------------------
    std::cout << "--- Example 3: Sun Crossing -18° (astronomical twilight) ---\n";
    std::cout << "(This is what siderust's sun::below_threshold uses internally)\n\n";

    const Geodetic obs   = ROQUE_DE_LOS_MUCHACHOS;
    const BodyTarget sun_target(Body::Sun);

    // Search for the exact MJD when Sun crosses -18° on MJD 60000
    const MJD mjd_search_start(59999.5);
    const MJD mjd_search_end(60000.5);

    // Sample to bracket
    double best_a = -1.0, best_b = -1.0;
    const double threshold = -18.0;
    for (int i = 0; i < 100; ++i) {
        const double t1 = mjd_search_start.value() + i * 0.01;
        const double t2 = t1 + 0.01;
        const double alt1 = sun_target.altitude_at(obs, MJD(t1)).value() - threshold;
        const double alt2 = sun_target.altitude_at(obs, MJD(t2)).value() - threshold;
        if (alt1 * alt2 < 0.0) {
            best_a = t1;
            best_b = t2;
            break;
        }
    }

    if (best_a > 0.0) {
        const double crossing_mjd = brent_find_root(best_a, best_b, 1e-9, [&](double t) {
            return sun_target.altitude_at(obs, MJD(t)).value() - threshold;
        });
        std::cout << "  Sun crosses -18° at MJD = " << std::fixed << std::setprecision(6)
                  << crossing_mjd << "\n";
        std::cout << "  Verify: alt at crossing = "
                  << std::setprecision(4) << sun_target.altitude_at(obs, MJD(crossing_mjd)).value()
                  << "° (≈ -18°)\n";
    } else {
        std::cout << "  No crossing found in search window.\n";
    }

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
