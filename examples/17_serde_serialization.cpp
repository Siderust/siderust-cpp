/**
 * @file 17_serde_serialization.cpp
 * @brief C++ port of siderust/examples/37_serde_serialization.rs
 *
 * [PLACEHOLDER] — Rust's `serde` serialization/deserialization framework has
 * no direct equivalent in the C++ wrapper.  JSON round-trips for siderust
 * types are **not yet available** in C++.
 *
 * This file shows how one could serialize the available C++ types manually
 * (as a demonstration), and documents what the Rust serde API looks like.
 *
 * Run with: cmake --build build --target serde_serialization_example
 */

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

// ---------------------------------------------------------------------------
// Minimal manual JSON helpers (C++ placeholder for serde)
// ---------------------------------------------------------------------------

static std::string mjd_to_json(const MJD &mjd) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6) << "{\"mjd\":" << mjd.value() << "}";
    return ss.str();
}

static std::string jd_to_json(const JulianDate &jd) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6) << "{\"jd\":" << jd.value() << "}";
    return ss.str();
}

static std::string geodetic_to_json(const Geodetic &g) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6)
       << "{\"lon_deg\":" << g.lon.value()
       << ",\"lat_deg\":" << g.lat.value()
       << ",\"alt_m\":"   << g.height.value() << "}";
    return ss.str();
}

static std::string direction_to_json(const spherical::direction::ICRS &d) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6)
       << "{\"ra_deg\":"  << d.ra().value()
       << ",\"dec_deg\":" << d.dec().value() << "}";
    return ss.str();
}

template <typename C, typename F, typename U>
static std::string position_to_json(const cartesian::Position<C, F, U> &p) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(9)
       << "{\"x\":" << p.x().value()
       << ",\"y\":" << p.y().value()
       << ",\"z\":" << p.z().value() << "}";
    return ss.str();
}

int main() {
    std::cout << "=== Serialization / Deserialization  [PLACEHOLDER] ===\n\n";

    // -------------------------------------------------------------------------
    // PLACEHOLDER: Rust serde API description
    // -------------------------------------------------------------------------
    std::cout << "NOTE: Rust serde JSON support is not yet bound in C++.\n";
    std::cout << "  // Rust:\n";
    std::cout << "  // use serde_json;\n";
    std::cout << "  // let jd: JulianDate = JulianDate::J2000();\n";
    std::cout << "  // let json = serde_json::to_string(&jd).unwrap();\n";
    std::cout << "  // let back: JulianDate = serde_json::from_str(&json).unwrap();\n";
    std::cout << "\n  // C++ (future API — planned with nlohmann/json or similar):\n";
    std::cout << "  // auto j = siderust::to_json(jd);\n";
    std::cout << "  // auto jd2 = siderust::from_json<JulianDate>(j);\n\n";

    // -------------------------------------------------------------------------
    // Manual serialization demo (available now)
    // -------------------------------------------------------------------------
    std::cout << "--- Manual JSON-like Serialization (C++ demo) ---\n\n";

    const JulianDate jd = JulianDate::J2000();
    const MJD        mjd(51544.5);
    const Geodetic   obs = ROQUE_DE_LOS_MUCHACHOS;
    const auto       mars_pos = ephemeris::mars_heliocentric(jd);

    std::cout << "JulianDate:     " << jd_to_json(jd) << "\n";
    std::cout << "MJD:            " << mjd_to_json(mjd) << "\n";
    std::cout << "Geodetic (Roque):\n  " << geodetic_to_json(obs) << "\n";
    // Sirius ICRS direction (J2000 coords; Star::direction() not yet bound)
    const spherical::direction::ICRS sirius_icrs(qtty::Degree(101.2871), qtty::Degree(-16.7161));
    std::cout << "Sirius ICRS dir:\n  " << direction_to_json(sirius_icrs) << "\n";
    std::cout << "Mars heliocentric:\n  " << position_to_json(mars_pos) << "\n";

    // -------------------------------------------------------------------------
    // Round-trip parse demo (manual)
    // -------------------------------------------------------------------------
    std::cout << "\n--- Manual Round-Trip Verification ---\n";

    const double jd_val = 2451545.0;
    const JulianDate jd2(jd_val);
    std::cout << "Serialized JD=" << jd_val << " → re-parsed JD=" << jd2.value() << " ✓\n";

    const double lat_val = obs.lat.value();
    const double lon_val = obs.lon.value();
    const double alt_val = obs.height.value();
    const Geodetic obs2 = Geodetic(lon_val, lat_val, alt_val);
    std::cout << "Re-parsed Geodetic lat=" << obs2.lat.value()
              << "\u00b0, lon=" << obs2.lon.value() << "\u00b0 \u2713\n";

    std::cout << "\nFull serde support: TODO — needs nlohmann/json or similar.\n";
    std::cout << "\n=== Example Complete (Placeholder) ===\n";
    return 0;
}
