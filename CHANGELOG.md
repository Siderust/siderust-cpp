# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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
