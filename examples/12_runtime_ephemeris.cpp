// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// @file 12_runtime_ephemeris.cpp
/// @brief Runtime JPL DE4xx ephemeris loading from a BSP file.
///
/// Demonstrates how to use `siderust::RuntimeEphemeris` to load a JPL
/// DE4xx BSP kernel at runtime and query planetary positions without
/// recompiling.  Unlike the built-in VSOP87 tables, the BSP file is
/// supplied by the user — no data is embedded in the library.
///
/// Demonstrates:
/// - Loading a BSP file from disk via `RuntimeEphemeris(path)`
/// - Loading from an in-memory buffer via `RuntimeEphemeris(ptr, len)`
/// - Querying Sun, Earth, and Moon positions at given epochs
/// - Graceful error handling via `siderust::DataLoadError`
///
/// Build & run:
///   cmake --build build-local --target 12_runtime_ephemeris_example
///   ./build-local/12_runtime_ephemeris_example /path/to/de440.bsp
///
/// If you don't have a BSP file, you can download DE440 (~32 MB) from:
///   https://naif.jpl.nasa.gov/pub/naif/generic_kernels/spk/planets/de440s.bsp

#include <siderust/siderust.hpp>

#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace siderust;

// ─── helpers ────────────────────────────────────────────────────────────────

template <typename Pos> static void print_pos(const char *label, const Pos &p) {
  std::cout << "  " << label << ' ' << p << '\n';
}

// ─── section 1: load from file ───────────────────────────────────────────────

static void demo_from_file(const std::string &bsp_path) {
  std::cout << "─────────────────────────────────────────────────────────────────\n"
            << "1) LOAD FROM BSP FILE\n"
            << "─────────────────────────────────────────────────────────────────\n";

  RuntimeEphemeris eph(bsp_path);

  Time<TT, JD> jd_j2000(2451545.0);

  std::cout << "  Positions at J2000.0 (2000-Jan-01.5 TDB):\n";
  print_pos("Sun   barycentric [AU] :", eph.sun_barycentric(jd_j2000));
  print_pos("Earth barycentric [AU] :", eph.earth_barycentric(jd_j2000));
  print_pos("Earth heliocentric [AU]:", eph.earth_heliocentric(jd_j2000));
  print_pos("Moon  geocentric  [km] :", eph.moon_geocentric(jd_j2000));

  Time<TT, JD> jd_now(2460841.5);
  std::cout << "\n  Positions at 2025-Jun-15.0 TDB (JD 2460841.5):\n";
  print_pos("Earth heliocentric [AU]:", eph.earth_heliocentric(jd_now));
  print_pos("Moon  geocentric  [km] :", eph.moon_geocentric(jd_now));
  std::cout << '\n';
}

// ─── section 2: move semantics ───────────────────────────────────────────────

static void demo_move_semantics(const std::string &bsp_path) {
  std::cout << "─────────────────────────────────────────────────────────────────\n"
            << "2) MOVE SEMANTICS\n"
            << "─────────────────────────────────────────────────────────────────\n";

  RuntimeEphemeris eph1(bsp_path);
  RuntimeEphemeris eph2(std::move(eph1));
  if (!eph1) {
    std::cout << "  eph1 is now empty (handle transferred to eph2) ✓\n";
  }
  if (eph2) {
    std::cout << "  eph2 owns the handle ✓\n";
  }

  Time<TT, JD> jd(2451545.0);
  std::cout << "  Earth-Sun distance via eph2: " << std::fixed << std::setprecision(8)
            << eph2.earth_heliocentric(jd).distance() << "\n\n";
}

// ─── section 3: error handling ───────────────────────────────────────────────

static void demo_error_handling() {
  std::cout << "─────────────────────────────────────────────────────────────────\n"
            << "3) ERROR HANDLING\n"
            << "─────────────────────────────────────────────────────────────────\n";

  const uint8_t bad_data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01, 0x02};
  try {
    RuntimeEphemeris eph(bad_data, sizeof(bad_data));
    std::cout << "  ERROR: expected DataLoadError was not thrown!\n";
  } catch (const DataLoadError &e) {
    std::cout << "  Caught expected DataLoadError: " << e.what() << '\n';
    std::cout << "  Error handling works correctly ✓\n";
  }

  try {
    RuntimeEphemeris eph("/does/not/exist/de440.bsp");
    std::cout << "  ERROR: expected DataLoadError was not thrown!\n";
  } catch (const DataLoadError &e) {
    std::cout << "  Caught expected DataLoadError for missing file: " << e.what() << '\n';
    std::cout << "  Error handling works correctly ✓\n";
  }
  std::cout << '\n';
}

// ─── section 4: load from memory buffer ─────────────────────────────────────

static void demo_from_bytes(const std::string &bsp_path) {
  std::cout << "─────────────────────────────────────────────────────────────────\n"
            << "4) LOAD FROM MEMORY BUFFER\n"
            << "─────────────────────────────────────────────────────────────────\n";

  std::ifstream file(bsp_path, std::ios::binary | std::ios::ate);
  if (!file) {
    std::cout << "  [Skipped — cannot reopen BSP file for memory demo]\n";
    return;
  }
  const auto fsize = static_cast<std::size_t>(file.tellg());
  file.seekg(0);
  std::vector<uint8_t> buf(fsize);
  file.read(reinterpret_cast<char *>(buf.data()), static_cast<std::streamsize>(fsize));

  std::cout << "  Loaded " << fsize << " bytes into memory\n";

  RuntimeEphemeris eph(buf.data(), buf.size());
  Time<TT, JD> jd(2451545.0);
  std::cout << "  Earth-Sun distance (from buffer): " << std::fixed << std::setprecision(8)
            << eph.earth_heliocentric(jd).distance() << "\n\n";
}

// ─── main ────────────────────────────────────────────────────────────────────

int main(int argc, char *argv[]) {
  std::cout << "╔══════════════════════════════════════════════════════════════╗\n"
            << "║        12 — Runtime JPL DE4xx Ephemeris Example             ║\n"
            << "╚══════════════════════════════════════════════════════════════╝\n\n";

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <path/to/de440.bsp>\n"
              << "\n"
              << "Tip: download the ~32 MB DE440s kernel from:\n"
              << "  "
                 "https://naif.jpl.nasa.gov/pub/naif/generic_kernels/spk/"
                 "planets/de440s.bsp\n"
              << "\n"
              << "Section 3 (error handling) runs without a BSP file:\n";

    demo_error_handling();
    return 1;
  }

  const std::string bsp_path = argv[1];

  try {
    demo_from_file(bsp_path);
    demo_move_semantics(bsp_path);
    demo_error_handling();
    demo_from_bytes(bsp_path);
  } catch (const std::exception &ex) {
    std::cerr << "Unhandled exception: " << ex.what() << '\n';
    return 1;
  }

  std::cout << "Done.\n";
  return 0;
}
