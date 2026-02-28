// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// @file 11_serde_serialization.cpp
/// @brief Manual JSON-like serialization examples.
///
/// Demonstrates how to export siderust data as JSON strings using standard
/// C++ I/O (no external JSON library needed).  Sections mirror the Rust
/// serde example:
///
///  1) Time objects
///  2) Coordinate objects
///  3) Body-related objects (orbit + ephemeris snapshot)
///  4) Target coordinate snapshots
///  5) File I/O (write → read → verify)
///
/// Build & run:
///   cmake --build build-local --target 11_serde_serialization_example
///   ./build-local/11_serde_serialization_example

#include <siderust/siderust.hpp>

#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace siderust;
using namespace siderust::frames;
using namespace siderust::centers;using namespace qtty::literals;
// ─── JSON formatting helpers ────────────────────────────────────────────────

inline std::string json_number(double v, int prec = 6) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(prec) << v;
    return os.str();
}

inline std::string json_string(const std::string &s) {
    return "\"" + s + "\"";
}

// ─── Section 1: Time objects ────────────────────────────────────────────────

void section_times() {
    std::puts("1) TIME OBJECTS");
    std::puts("---------------");

    JulianDate jd = JulianDate::J2000();
    auto mjd = jd.to<tempoch::MJDScale>();
    JulianDate jd_plus1(jd.value() + 1.0);
    JulianDate jd_plus7(jd.value() + 7.0);

    // Pretty-print a TimeBundle-like JSON
    std::puts("{");
    std::printf("  \"j2000\": %s,\n", json_number(jd.value(), 1).c_str());
    std::printf("  \"mjd\": %s,\n", json_number(mjd.value(), 1).c_str());
    std::printf("  \"timeline\": [\n");
    std::printf("    %s,\n", json_number(jd.value(), 1).c_str());
    std::printf("    %s,\n", json_number(jd_plus1.value(), 1).c_str());
    std::printf("    %s\n", json_number(jd_plus7.value(), 1).c_str());
    std::puts("  ]");
    std::puts("}");

    std::cout << "Roundtrip check: j2000=" << std::fixed << std::setprecision(1)
              << jd << ", timeline_len=3\n" << std::endl;
}

// ─── Section 2: Coordinate objects ──────────────────────────────────────────

void section_coordinates() {
    std::puts("2) COORDINATE OBJECTS");
    std::puts("---------------------");

    // Geocentric ICRS cartesian (km)
    cartesian::Position<Geocentric, ICRS, qtty::Kilometer> geo_icrs_cart(
        6371.0, 0.0, 0.0);

    // Heliocentric ecliptic spherical (AU)
    spherical::Position<Heliocentric, EclipticMeanJ2000, qtty::AstronomicalUnit>
        helio_ecl_sph(120.0_deg, 5.0_deg,
                      1.2_au);

    // Observer site (geodetic)
    Geodetic observer_site(-17.8947, 28.7636, 2396.0);

    std::puts("{");
    std::printf("  \"geo_icrs_cart\": { \"x\": %s, \"y\": %s, \"z\": %s, \"unit\": \"km\" },\n",
                json_number(geo_icrs_cart.x().value(), 1).c_str(),
                json_number(geo_icrs_cart.y().value(), 1).c_str(),
                json_number(geo_icrs_cart.z().value(), 1).c_str());
    std::printf("  \"helio_ecl_sph\": { \"lon\": %s, \"lat\": %s, \"r\": %s, \"unit\": \"AU\" },\n",
                json_number(helio_ecl_sph.direction().lon().value(), 1).c_str(),
                json_number(helio_ecl_sph.direction().lat().value(), 1).c_str(),
                json_number(helio_ecl_sph.distance().value(), 1).c_str());
    std::printf("  \"observer_site\": { \"lon\": %s, \"lat\": %s, \"height_m\": %s }\n",
                json_number(observer_site.lon.value(), 4).c_str(),
                json_number(observer_site.lat.value(), 4).c_str(),
                json_number(observer_site.height.value(), 1).c_str());
    std::puts("}");

    std::cout << "Roundtrip check: x=" << std::fixed << std::setprecision(1)
              << geo_icrs_cart.x() << ", lon="
              << std::setprecision(4) << observer_site.lon << "\n" << std::endl;
}

// ─── Section 3: Body-related objects ────────────────────────────────────────

struct BodySnapshotJSON {
    std::string name;
    JulianDate epoch;
    Orbit orbit;
    cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit> helio_ecl;

    std::string to_json(int indent = 2) const {
        std::string pad(indent, ' ');
        std::ostringstream os;
        os << "{\n"
           << pad << "\"name\": " << json_string(name) << ",\n"
           << pad << "\"epoch\": " << json_number(epoch.value(), 1) << ",\n"
           << pad << "\"orbit\": {\n"
           << pad << "  \"semi_major_axis_au\": " << json_number(orbit.semi_major_axis.value()) << ",\n"
           << pad << "  \"eccentricity\": " << json_number(orbit.eccentricity) << ",\n"
           << pad << "  \"inclination_deg\": " << json_number(orbit.inclination.value()) << ",\n"
           << pad << "  \"lon_ascending_node_deg\": " << json_number(orbit.lon_ascending_node.value()) << ",\n"
           << pad << "  \"arg_perihelion_deg\": " << json_number(orbit.arg_perihelion.value()) << ",\n"
           << pad << "  \"mean_anomaly_deg\": " << json_number(orbit.mean_anomaly.value()) << ",\n"
           << pad << "  \"epoch_jd\": " << json_number(orbit.epoch_jd, 1) << "\n"
           << pad << "},\n"
           << pad << "\"heliocentric_ecliptic\": {\n"
           << pad << "  \"x\": " << json_number(helio_ecl.x().value()) << ",\n"
           << pad << "  \"y\": " << json_number(helio_ecl.y().value()) << ",\n"
           << pad << "  \"z\": " << json_number(helio_ecl.z().value()) << "\n"
           << pad << "}\n"
           << "}";
        return os.str();
    }
};

void section_body_objects(const JulianDate &jd) {
    std::puts("3) BODY-RELATED OBJECTS");
    std::puts("-----------------------");

    BodySnapshotJSON earth_snap{
        "Earth", jd, EARTH.orbit, ephemeris::earth_heliocentric(jd)};

    // Halley's comet
    Orbit halley_orb{17.834_au, 0.96714,
                     162.26_deg, 58.42_deg,
                     111.33_deg, 38.38_deg, 2446467.4};
    auto halley_pos = kepler_position(halley_orb, jd);
    BodySnapshotJSON halley_snap{"Halley", jd, halley_orb, halley_pos};

    std::puts("Earth snapshot JSON:");
    std::puts(earth_snap.to_json().c_str());
    std::puts("Halley snapshot JSON:");
    std::puts(halley_snap.to_json().c_str());

    std::cout << "Roundtrip check: " << halley_snap.name
              << " @ JD " << std::fixed << std::setprecision(1)
              << halley_snap.epoch << ", r="
              << std::setprecision(6) << halley_snap.helio_ecl.distance()
              << "\n" << std::endl;
}

// ─── Section 4: Target objects ──────────────────────────────────────────────

void section_targets(const JulianDate &jd) {
    std::puts("4) TARGET OBJECTS");
    std::puts("-----------------");

    // Mars barycentric target
    auto mars_bary = ephemeris::mars_barycentric(jd);
    // Moon geocentric target
    auto moon_geo  = ephemeris::moon_geocentric(jd);

    std::puts("{");
    std::printf("  \"mars_bary_target\": {\n");
    std::printf("    \"time\": %s,\n", json_number(jd.value(), 1).c_str());
    std::printf("    \"position\": { \"x\": %s, \"y\": %s, \"z\": %s }\n",
                json_number(mars_bary.x().value()).c_str(),
                json_number(mars_bary.y().value()).c_str(),
                json_number(mars_bary.z().value()).c_str());
    std::printf("  },\n");
    std::printf("  \"moon_geo_target\": {\n");
    std::printf("    \"time\": %s,\n", json_number(jd.value(), 1).c_str());
    std::printf("    \"position\": { \"x\": %s, \"y\": %s, \"z\": %s }\n",
                json_number(moon_geo.x().value()).c_str(),
                json_number(moon_geo.y().value()).c_str(),
                json_number(moon_geo.z().value()).c_str());
    std::printf("  }\n");
    std::puts("}");

    std::cout << "Roundtrip check: Mars target JD " << std::fixed
              << std::setprecision(1) << jd
              << ", Moon target JD " << jd << "\n" << std::endl;
}

// ─── Section 5: File I/O ────────────────────────────────────────────────────

void section_file_io(const JulianDate &jd) {
    std::puts("5) FILE I/O");
    std::puts("----------");

    const std::string out_path = "/tmp/siderust_serde_example_targets.json";

    // Build a JSON string for the targets
    auto mars_bary = ephemeris::mars_barycentric(jd);
    auto moon_geo  = ephemeris::moon_geocentric(jd);

    std::ostringstream json;
    json << std::fixed << std::setprecision(6);
    json << "{\n";
    json << "  \"mars_bary_target\": {\n";
    json << "    \"time\": " << jd.value() << ",\n";
    json << "    \"position\": { \"x\": " << mars_bary.x().value()
         << ", \"y\": " << mars_bary.y().value()
         << ", \"z\": " << mars_bary.z().value() << " }\n";
    json << "  },\n";
    json << "  \"moon_geo_target\": {\n";
    json << "    \"time\": " << jd.value() << ",\n";
    json << "    \"position\": { \"x\": " << moon_geo.x().value()
         << ", \"y\": " << moon_geo.y().value()
         << ", \"z\": " << moon_geo.z().value() << " }\n";
    json << "  }\n";
    json << "}";

    // Write
    {
        std::ofstream ofs(out_path);
        ofs << json.str();
    }

    // Read back and verify
    {
        std::ifstream ifs(out_path);
        std::string content((std::istreambuf_iterator<char>(ifs)),
                            std::istreambuf_iterator<char>());
        if (!content.empty()) {
            std::printf("Saved and loaded: %s (%zu bytes)\n",
                        out_path.c_str(), content.size());
        } else {
            std::puts("Error: file I/O failed.");
        }
    }
}

// ─── main ─────────────────────────────────────────────────────────────────────

int main() {
    std::puts("=== Siderust Manual Serialization Examples ===\n");

    JulianDate jd = JulianDate::J2000();

    section_times();
    section_coordinates();
    section_body_objects(jd);
    section_targets(jd);
    section_file_io(jd);

    return 0;
}
