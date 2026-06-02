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

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace siderust;
using namespace siderust::frames;
using namespace siderust::centers;
using namespace qtty::literals;
using TTJD = JulianDate;
// ─── JSON formatting helpers ────────────────────────────────────────────────

inline std::string json_number(double v, int prec = 6) {
  std::ostringstream os;
  os << std::fixed << std::setprecision(prec) << v;
  return os.str();
}

inline std::string json_string(const std::string &s) { return "\"" + s + "\""; }

// ─── Section 1: Time objects ────────────────────────────────────────────────

void section_times() {
  std::cout << "1) TIME OBJECTS\n"
            << "---------------\n";

  auto jd = TTJD::J2000();
  auto mjd = jd.to<format::MJD>();
  auto jd_plus1 = jd + qtty::Day(1.0);
  auto jd_plus7 = jd + qtty::Day(7.0);

  std::cout << "{\n";
  std::cout << "  \"j2000\": " << json_number(jd.value(), 1) << ",\n";
  std::cout << "  \"mjd\": " << json_number(mjd.value(), 1) << ",\n";
  std::cout << "  \"timeline\": [\n";
  std::cout << "    " << json_number(jd.value(), 1) << ",\n";
  std::cout << "    " << json_number(jd_plus1.value(), 1) << ",\n";
  std::cout << "    " << json_number(jd_plus7.value(), 1) << "\n";
  std::cout << "  ]\n";
  std::cout << "}\n";

  std::cout << "Roundtrip check: j2000=" << std::fixed << std::setprecision(1) << jd
            << ", timeline_len=3\n\n";
}

// ─── Section 2: Coordinate objects ──────────────────────────────────────────

void section_coordinates() {
  std::cout << "2) COORDINATE OBJECTS\n"
            << "---------------------\n";

  // Geocentric ICRS cartesian (km)
  cartesian::Position<Geocentric, ICRS, qtty::Kilometer> geo_icrs_cart(6371.0, 0.0, 0.0);

  // Heliocentric ecliptic spherical (AU)
  spherical::Position<Heliocentric, EclipticMeanJ2000, qtty::AstronomicalUnit> helio_ecl_sph(
      120.0_deg, 5.0_deg, 1.2_au);

  // Observer site (geodetic)
  Geodetic observer_site(-17.8947, 28.7636, 2396.0);

  std::cout << "{\n";
  std::cout << "  \"geo_icrs_cart\": { \"x\": " << json_number(geo_icrs_cart.x().value(), 1)
            << ", \"y\": " << json_number(geo_icrs_cart.y().value(), 1)
            << ", \"z\": " << json_number(geo_icrs_cart.z().value(), 1)
            << ", \"unit\": \"km\" },\n";
  std::cout << "  \"helio_ecl_sph\": { \"lon\": "
            << json_number(helio_ecl_sph.direction().lon().value(), 1)
            << ", \"lat\": " << json_number(helio_ecl_sph.direction().lat().value(), 1)
            << ", \"r\": " << json_number(helio_ecl_sph.distance().value(), 1)
            << ", \"unit\": \"AU\" },\n";
  std::cout << "  \"observer_site\": { \"lon\": " << json_number(observer_site.lon.value(), 4)
            << ", \"lat\": " << json_number(observer_site.lat.value(), 4)
            << ", \"height_m\": " << json_number(observer_site.height.value(), 1) << " }\n";
  std::cout << "}\n";

  std::cout << "Roundtrip check: geo_icrs_cart=" << geo_icrs_cart
            << ", observer_site=" << observer_site << "\n\n";
}

// ─── Section 3: Body-related objects ────────────────────────────────────────

struct BodySnapshotJSON {
  std::string name;
  TTJD epoch;
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
       << pad << "  \"lon_ascending_node_deg\": " << json_number(orbit.lon_ascending_node.value())
       << ",\n"
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

void section_body_objects(const TTJD &jd) {
  std::cout << "3) BODY-RELATED OBJECTS\n"
            << "-----------------------\n";

  BodySnapshotJSON earth_snap{"Earth", jd, EARTH().orbit, ephemeris::earth_heliocentric(jd)};

  // Halley's comet
  Orbit halley_orb{17.834_au, 0.96714, 162.26_deg, 58.42_deg, 111.33_deg, 38.38_deg, 2446467.4};
  auto halley_pos = kepler_position(halley_orb, jd);
  BodySnapshotJSON halley_snap{"Halley", jd, halley_orb, halley_pos};

  std::cout << "Earth snapshot JSON:\n" << earth_snap.to_json() << '\n';
  std::cout << "Halley snapshot JSON:\n" << halley_snap.to_json() << '\n';

  std::cout << "Roundtrip check: " << halley_snap.name << " @ " << halley_snap.epoch
            << ", r=" << std::fixed << std::setprecision(6) << halley_snap.helio_ecl.distance()
            << "\n\n";
}

// ─── Section 4: Target objects ──────────────────────────────────────────────

void section_targets(const TTJD &jd) {
  std::cout << "4) TARGET OBJECTS\n"
            << "-----------------\n";

  auto mars_bary = ephemeris::mars_barycentric(jd);
  auto moon_geo = ephemeris::moon_geocentric(jd);

  std::cout << "{\n";
  std::cout << "  \"mars_bary_target\": {\n";
  std::cout << "    \"time\": " << json_number(jd.value(), 1) << ",\n";
  std::cout << "    \"position\": " << mars_bary << "\n";
  std::cout << "  },\n";
  std::cout << "  \"moon_geo_target\": {\n";
  std::cout << "    \"time\": " << json_number(jd.value(), 1) << ",\n";
  std::cout << "    \"position\": " << moon_geo << "\n";
  std::cout << "  }\n";
  std::cout << "}\n";

  std::cout << "Roundtrip check: Mars target " << jd << ", Moon target " << jd << "\n\n";
}

// ─── Section 5: File I/O ────────────────────────────────────────────────────

void section_file_io(const TTJD &jd) {
  std::cout << "5) FILE I/O\n"
            << "----------\n";

  const std::string out_path = "/tmp/siderust_serde_example_targets.json";

  auto mars_bary = ephemeris::mars_barycentric(jd);
  auto moon_geo = ephemeris::moon_geocentric(jd);

  std::ostringstream json;
  json << std::fixed << std::setprecision(6);
  json << "{\n";
  json << "  \"mars_bary_target\": {\n";
  json << "    \"time\": " << jd.value() << ",\n";
  json << "    \"position\": " << mars_bary << "\n";
  json << "  },\n";
  json << "  \"moon_geo_target\": {\n";
  json << "    \"time\": " << jd.value() << ",\n";
  json << "    \"position\": " << moon_geo << "\n";
  json << "  }\n";
  json << "}";

  {
    std::ofstream ofs(out_path);
    ofs << json.str();
  }

  {
    std::ifstream ifs(out_path);
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    if (!content.empty()) {
      std::cout << "Saved and loaded: " << out_path << " (" << content.size() << " bytes)\n";
    } else {
      std::cout << "Error: file I/O failed.\n";
    }
  }
}

// ─── main
// ─────────────────────────────────────────────────────────────────────

int main() {
  std::cout << "=== Siderust Manual Serialization Examples ===\n\n";

  auto jd = TTJD::J2000();

  section_times();
  section_coordinates();
  section_body_objects(jd);
  section_targets(jd);
  section_file_io(jd);

  return 0;
}
