# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.6.0] - 2026/06/02

### Added

- `tests/installed-consumer/` fixture and `.github/workflows/ci-installed-consumer.yml`
  workflow exercising the full installed-package consumer path:
  configure → build → install to a staging prefix → external CMake
  project does `find_package(siderust_cpp REQUIRED)`, includes
  `<siderust/siderust.hpp>`, links `siderust::siderust_cpp`, and runs a
  runtime smoke binary against the bundled `libsiderust_ffi.so`.
- Top-level guards `PROJECT_IS_TOP_LEVEL` and explicit options
  `SIDERUST_CPP_BUILD_TESTS`, `SIDERUST_CPP_BUILD_EXAMPLES`,
  `SIDERUST_CPP_INSTALL`, `SIDERUST_CPP_ENABLE_PACKAGING` so siderust-cpp
  is safe to consume via `add_subdirectory(...)` without dragging in
  GoogleTest, examples, install rules, or CPack.
- `siderust::siderust_cpp` ALIAS target so build-tree consumers can use
  the same namespaced target name as installed consumers.
- Pointer to a Phase-B work plan and public-API coverage matrix for the
  upcoming dynamics-and-pod expansion (see plan.md in the session
  workspace).

### Changed

- `.gitmodules` now uses HTTPS URLs (`https://github.com/Siderust/...`)
  for both the `siderust` and `tempoch-cpp` submodules so public clones
  and CI jobs without SSH credentials succeed.
- `cmake/siderust_cppConfig.cmake.in` rewritten:
  - calls `find_dependency(qtty_cpp)` and `find_dependency(tempoch_cpp)`,
  - defines `siderust::siderust_ffi` as a SHARED IMPORTED target that
    points at the bundled `libsiderust_ffi.so` in the install prefix,
    so consumers linking `siderust::siderust_cpp` resolve FFI symbols
    transitively without manually setting `LD_LIBRARY_PATH` paths,
  - includes the exported `siderust_cppTargets.cmake`,
  - exposes `siderust_cpp_INCLUDE_DIRS` / `siderust_cpp_LIBRARIES`
    legacy variables for non-target-based consumers.
- Top-level `CMakeLists.txt`:
  - bumped minimum CMake to 3.21 (for `PROJECT_IS_TOP_LEVEL`),
  - uses `GNUInstallDirs` (`CMAKE_INSTALL_INCLUDEDIR` /
    `CMAKE_INSTALL_LIBDIR`) instead of hard-coded `include/` `lib/`,
  - install/packaging/tests/examples are all gated on the new options.
- CPack DEB/RPM dependencies updated to the real upstream versions
  pulled in by the current `siderust` submodule:
  `qtty-cpp >= 0.8`, `tempoch-cpp >= 0.6` (was 0.4.2 / 0.3.1).
- `Dockerfile.prod` rust-builder stage now:
  - sets `CARGO_TARGET_DIR=/src/tempoch-cpp/tempoch/tempoch-ffi/target`
    when building `tempoch-ffi` so the artifact lands at the path the
    cpp-builder stage expects,
  - builds `qtty-ffi` with `cargo build -p qtty-ffi` from the qtty
    workspace root so the artifact lands in
    `/src/tempoch-cpp/qtty-cpp/qtty/target/release/` (not the previously
    incorrect `qtty-ffi/target/release/` per-crate path that does not
    exist for workspace members),
  - source paths in the COPY blocks now match what the rust-builder
    stage actually produces.

### Removed

- `include/siderust/constops.hpp`, `include/siderust/constops.h`, and
  `tests/test_constops.cpp`. The constops Rust crate does not exist in
  this workspace or in `siderust-ffi`, so the header declared symbols
  that no library exported. Any consumer that included `constops.hpp`
  would fail to link. The stub is gone; per the user-stated policy, no
  deprecated/legacy code remains.
- `|| true` masking on the `cp` of Rust shared libraries to
  `build/staging/lib/` in `.github/workflows/ci-package.yml`. Missing
  shared libraries are now a hard CI failure rather than a silently
  broken package.

## [0.5.0] - 2026-05-18

### Removed

- Dropped the C++ satellite/dynamics adapter surface, including
  `include/siderust/dynamics.hpp`, the vendored `siderust-ffi` dynamics
  exports, and the associated C++ dynamics tests (`tests/test_dynamics.cpp`).
  Specifying the `satellite` CMake feature now produces a `FATAL_ERROR`.
- Removed backward-compatible `(start, end)` two-argument overloads from
  `altitude.hpp`, `azimuth.hpp`, `lunar_phase.hpp`, `subject.hpp`, and
  `target.hpp`.  Callers must construct a `Period(start, end)` explicitly and
  pass it to the `Period`-taking overload.
- Removed `#include <siderust/dynamics.hpp>` from the umbrella
  `include/siderust/siderust.hpp` header.

### Changed

- Submodule `siderust` advanced to v0.8.0 (from v0.7.0): brings qtty 0.8,
  tempoch 0.6, `siderust::JulianDate` / `ModifiedJulianDate` crate-root
  re-exports, and removal of the FFI dynamics layer.
- Submodule `tempoch-cpp` advanced to v0.5.0 (from v0.4.x).

## [0.4.0] - 2026-05-15

### Added

- `include/siderust/constops.hpp` and generated `include/siderust/constops.h` with JSON-oriented wrappers for the constops FFI surface, including route constants and ground-asset ID helpers.

### Changed

- `include/siderust/time.hpp` now exposes the v1 time surface: `UTC`, TT-default `JulianDate` / `MJD` / `Period`, explicit `Time<scale::S>`, and named mixed-scale aliases such as `UT1JulianDate`.
- Public headers, examples, tests, README, and Doxygen docs now default to the astronomy-facing TT aliases and the direct civil-time constructors `JulianDate::from_utc(...)` / `MJD::from_utc(...)`.
- Updated the vendored `siderust` and `tempoch-cpp` submodules to the current snapshots used by this branch, including the upstream typed-time ergonomics restored for v1.

### Fixed

- `cartesian::Position::to_spherical()` now normalizes longitude into `[0°, 360°)` instead of returning negative values for points in the negative-Y half-plane.

## [0.3.1] - 2026-05-11

### Fixed

- `CMakeLists.txt`: guarded all `install()` rules, `CMakePackageConfigHelpers`
  calls, tests, examples, and GoogleTest integration inside
  `if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)` so that siderust-cpp
  artifacts are no longer included in parent packages when siderust-cpp is used
  as a CMake subdirectory.  This eliminates file-conflict errors such as
  `trying to overwrite '/usr/include/siderust/ffi_core.hpp'` when multiple
  packages are installed simultaneously.

### Changed

- CPack DEB dependencies now carry minimum version constraints:
  `qtty-cpp (>= 0.4.2)` and `tempoch-cpp (>= 0.3.1)`.
- CPack RPM dependencies updated to `qtty-cpp >= 0.4.2, tempoch-cpp >= 0.3.1`.

## [0.3.0] - 2026-05-09

### Added

- `outside_azimuth_range` C++ wrappers in `include/siderust/azimuth.hpp` for Sun, Moon, and Star subjects, mirroring the existing `in_azimuth_range` surface and backed by the new `siderust_outside_azimuth_range` FFI function.
- `cheby` and `qtty` as git submodules, required by the `siderust` Rust workspace patch overrides when building the FFI from source.

### Changed

- Advance `siderust` submodule to `v0.7.0`: brings `siderust_outside_azimuth_range` FFI function, `SIDERUST_STATUS_T_OUT_OF_RANGE` (13) and `SIDERUST_STATUS_T_NO_EOP_DATA` (14) enum values, and all upstream dependency updates (qtty 0.7.0, affn 0.6.2).
- Updated `tempoch::*Scale` type references in examples `07_moon_properties.cpp`, `10_time_periods.cpp`, and `11_serde_serialization.cpp` to the new `tempoch::scales::*` namespace introduced in tempoch-cpp v0.2.1.
- `CMakeLists.txt`: set `INSTALL_GTEST=OFF` and `INSTALL_GMOCK=OFF` before `FetchContent_MakeAvailable(googletest)` to fix `cmake --install` / CPack failures caused by GoogleTest adding spurious install rules.

### Fixed

- `.value()` removed from `Displacement` arithmetic operators (`+`, `-`, unary `-`, `*`) and `Position` arithmetic operators in `include/siderust/coordinates/cartesian.hpp`; qtty's own operator overloads are used instead.
- clang-format violations across `examples/` and `tests/` (automated via `clang-format -i`).

## [0.2.0] - 2026-05-08

### Added

- New orbit binding header `include/siderust/orbit.hpp` with first-class C++ wrappers for `KeplerianOrbit`, `MeanMotionOrbit`, `ConicOrbit`, `ConicKind`, and move-only `PreparedOrbit`.
- New astronomical context header `include/siderust/astro_context.hpp` with Earth-orientation model tags `Iau2000A`, `Iau2000B`, `Iau2006`, and `Iau2006A`.
- New model-aware coordinate transform entry points on the C++ API via `to_frame_with(...)` and `to_horizontal_with(...)` for the typed spherical/cartesian wrappers.
- New C++ examples `examples/14_nutation_models.cpp` and `examples/15_orbit_models.cpp`.
- New Rust mirror example `siderust/examples/15_orbit_models.rs`.
- New C++ orbit regression coverage in `tests/test_orbits.cpp`.
- `AstroContext::from_default_ffi()` static factory — round-trips through the Rust library's built-in default to discover its preferred EOP model.
- `detail::OwnedFfiContext` default constructor (calls `siderust_context_create_default`) and `model()` accessor (calls `siderust_context_get_model`).
- `Direction<F>::to_horizontal_precise(jd_tt, jd_ut1, obs)` — high-precision horizontal transform with separate TT and UT1 epochs.
- `RuntimeEphemeris::earth_barycentric_velocity(jd)` — Earth barycentric velocity (AU/day) from a loaded JPL DE kernel.
- New `CartesianVelocity` struct (vx, vy, vz, frame) returned by `earth_barycentric_velocity`.
- `DirectionTarget::epoch_jd()` and `DirectionTarget::data()` — accessors that query the FFI handle directly.
- New `ProperMotionTarget` class wrapping `siderust_generic_target_create_icrs_with_pm`; exposes the full `Target`/`Trackable` API including proper-motion–corrected altitude/azimuth queries.
- New `RaConvention` enum (`MuAlpha`, `MuAlphaStar`) companion to `ProperMotionTarget`.
- New `include/siderust/twilight.hpp` header with `TwilightPhase` enum and `twilight_phase(qtty::Degree)` / `twilight_phase(qtty::Radian)` free functions.
- `OutOfRangeError` and `NoEopDataError` exception types; `check_status` now handles `SIDERUST_STATUS_T_OUT_OF_RANGE` (13) and `SIDERUST_STATUS_T_NO_EOP_DATA` (14).
- New test files `tests/test_context.cpp` and `tests/test_twilight.cpp`; extended `test_ephemeris.cpp`, `test_coordinates.cpp`, and `test_subject.cpp` with coverage for all new API surface.
- Examples `01_basic_coordinates` and `02_coordinate_transformations` are now registered as CMake build targets (they existed on disk but were not wired up).
- Multi-stage production Docker image `Dockerfile.prod` (rust-builder → cpp-builder → minimal runtime).
- CPack configuration for `.deb` and `.rpm` package generation.
- CI: `push` triggers on `main` and `stars/dev` branches; tag-based triggers on `v*`; new `package` job generating and uploading DEB/RPM artifacts; new `release` job creating a GitHub Release on version tags.

### Changed

- `Orbit` is now a compatibility alias to `KeplerianOrbit`; new C++ APIs and examples use the explicit elliptic-orbit name.
- Orbit propagation in the C++ facade now routes through the newer FFI orbit APIs, including explicit-center Kepler propagation and prepared-orbit handles.
- C++ coordinate transforms can now opt into non-default nutation/precession models without changing the existing default-path APIs.
- Build metadata and docs now register the new orbit/nutation examples and describe the orbit header as part of the public adapter surface.
- CI `permissions.contents` elevated to `write` to allow GitHub Release creation.
- Project version bumped to `0.2.0`.
- `siderust` submodule advanced to `stars-integration` branch (siderust-ffi v0.4.1).

## [0.1.0] - 2026-03-08

- Initial C++ bindings and FFI wrappers
- CMake build, headers under `include/`, examples in `examples/`, and basic tests in `tests/`
- Project README, CI scripts, and packaging metadata
