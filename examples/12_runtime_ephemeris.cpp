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

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace siderust;

// ─── helpers ────────────────────────────────────────────────────────────────

/// Print XYZ components and distance for any Cartesian position type.
template <typename Pos> static void print_pos(const char *label, const Pos &p) {
  std::cout << "  " << label << "\n"
            << "    x = " << std::fixed << std::setprecision(8) << p.x() << "\n"
            << "    y = " << std::fixed << std::setprecision(8) << p.y() << "\n"
            << "    z = " << std::fixed << std::setprecision(8) << p.z() << "\n"
            << "    |r| = " << std::fixed << std::setprecision(8) << p.distance() << "\n";
}

// ─── section 1: load from file ───────────────────────────────────────────────

/// Demonstrates `RuntimeEphemeris(path)` — basic file-based loading.
static void demo_from_file(const std::string &bsp_path) {
  std::puts("─────────────────────────────────────────────────────────────────");
  std::puts("1) LOAD FROM BSP FILE");
  std::puts("─────────────────────────────────────────────────────────────────");

  RuntimeEphemeris eph(bsp_path); // throws DataLoadError on failure

  // J2000.0 epoch
  JulianDate jd_j2000(2451545.0);

  std::puts("  Positions at J2000.0 (2000-Jan-01.5 TDB):");
  print_pos("Sun   barycentric [AU] :", eph.sun_barycentric(jd_j2000));
  print_pos("Earth barycentric [AU] :", eph.earth_barycentric(jd_j2000));
  print_pos("Earth heliocentric [AU]:", eph.earth_heliocentric(jd_j2000));
  print_pos("Moon  geocentric  [km] :", eph.moon_geocentric(jd_j2000));

  // A second epoch: 2025-Jun-15.0 TDB
  JulianDate jd_now(2460841.5);
  std::puts("\n  Positions at 2025-Jun-15.0 TDB (JD 2460841.5):");
  print_pos("Earth heliocentric [AU]:", eph.earth_heliocentric(jd_now));
  print_pos("Moon  geocentric  [km] :", eph.moon_geocentric(jd_now));
  std::puts("");
}

// ─── section 2: move semantics ───────────────────────────────────────────────

/// Shows that `RuntimeEphemeris` is move-only and that moves transfer the
/// underlying handle (no copy, no double-free).
static void demo_move_semantics(const std::string &bsp_path) {
  std::puts("─────────────────────────────────────────────────────────────────");
  std::puts("2) MOVE SEMANTICS");
  std::puts("─────────────────────────────────────────────────────────────────");

  RuntimeEphemeris eph1(bsp_path);

  // Move-construct into eph2; eph1 is now empty.
  RuntimeEphemeris eph2(std::move(eph1));
  if (!eph1) { // operator bool() — eph1 no longer owns the handle
    std::puts("  eph1 is now empty (handle transferred to eph2) ✓");
  }
  if (eph2) {
    std::puts("  eph2 owns the handle ✓");
  }

  JulianDate jd(2451545.0);
  std::cout << "  Earth-Sun distance via eph2: " << std::fixed << std::setprecision(8)
            << eph2.earth_heliocentric(jd).distance() << "\n";
  std::puts("");
}

// ─── section 3: error handling ───────────────────────────────────────────────

/// Demonstrates the `DataLoadError` exception thrown for invalid data.
static void demo_error_handling() {
  std::puts("─────────────────────────────────────────────────────────────────");
  std::puts("3) ERROR HANDLING");
  std::puts("─────────────────────────────────────────────────────────────────");

  // Attempt to load garbage bytes — should throw DataLoadError.
  const uint8_t bad_data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01, 0x02};
  try {
    RuntimeEphemeris eph(bad_data, sizeof(bad_data));
    std::puts("  ERROR: expected DataLoadError was not thrown!");
  } catch (const DataLoadError &e) {
    std::cout << "  Caught expected DataLoadError: " << e.what() << "\n";
    std::puts("  Error handling works correctly ✓");
  }

  // Attempt to load a non-existent file path.
  try {
    RuntimeEphemeris eph("/does/not/exist/de440.bsp");
    std::puts("  ERROR: expected DataLoadError was not thrown!");
  } catch (const DataLoadError &e) {
    std::cout << "  Caught expected DataLoadError for missing file: " << e.what() << "\n";
    std::puts("  Error handling works correctly ✓");
  }
  std::puts("");
}

// ─── section 4: load from memory buffer ─────────────────────────────────────

/// Demonstrates reading a BSP file into a `std::vector<uint8_t>` and
/// passing the buffer directly to `RuntimeEphemeris(ptr, len)`.
static void demo_from_bytes(const std::string &bsp_path) {
  std::puts("─────────────────────────────────────────────────────────────────");
  std::puts("4) LOAD FROM MEMORY BUFFER");
  std::puts("─────────────────────────────────────────────────────────────────");

  // Read entire file into memory.
  std::ifstream file(bsp_path, std::ios::binary | std::ios::ate);
  if (!file) {
    std::puts("  [Skipped — cannot reopen BSP file for memory demo]");
    return;
  }
  const auto fsize = static_cast<std::size_t>(file.tellg());
  file.seekg(0);
  std::vector<uint8_t> buf(fsize);
  file.read(reinterpret_cast<char *>(buf.data()), static_cast<std::streamsize>(fsize));

  std::cout << "  Loaded " << fsize << " bytes into memory\n";

  RuntimeEphemeris eph(buf.data(), buf.size());
  JulianDate jd(2451545.0);
  std::cout << "  Earth-Sun distance (from buffer): " << std::fixed << std::setprecision(8)
            << eph.earth_heliocentric(jd).distance() << "\n";
  std::puts("");
}

// ─── main ────────────────────────────────────────────────────────────────────

int main(int argc, char *argv[]) {
  std::puts("╔══════════════════════════════════════════════════════════════╗");
  std::puts("║        12 — Runtime JPL DE4xx Ephemeris Example             ║");
  std::puts("╚══════════════════════════════════════════════════════════════╝");
  std::puts("");

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <path/to/de440.bsp>\n"
              << "\n"
              << "Tip: download the ~32 MB DE440s kernel from:\n"
              << "  "
                 "https://naif.jpl.nasa.gov/pub/naif/generic_kernels/spk/"
                 "planets/de440s.bsp\n"
              << "\n"
              << "Section 3 (error handling) runs without a BSP file:\n";

    // Still run the error-handling section so the example is useful even
    // without a BSP file.
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
    std::cerr << "Unhandled exception: " << ex.what() << "\n";
    return 1;
  }

  std::puts("Done.");
  return 0;
}
