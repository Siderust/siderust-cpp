# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- New orbit binding header `include/siderust/orbit.hpp` with first-class C++ wrappers for `KeplerianOrbit`, `MeanMotionOrbit`, `ConicOrbit`, `ConicKind`, and move-only `PreparedOrbit`.
- New astronomical context header `include/siderust/astro_context.hpp` with Earth-orientation model tags `Iau2000A`, `Iau2000B`, `Iau2006`, and `Iau2006A`.
- New model-aware coordinate transform entry points on the C++ API via `to_frame_with(...)` and `to_horizontal_with(...)` for the typed spherical/cartesian wrappers.
- New C++ examples `examples/14_nutation_models.cpp` and `examples/15_orbit_models.cpp`.
- New Rust mirror example `siderust/examples/15_orbit_models.rs`.
- New C++ orbit regression coverage in `tests/test_orbits.cpp`.

### Changed

- `Orbit` is now a compatibility alias to `KeplerianOrbit`; new C++ APIs and examples use the explicit elliptic-orbit name.
- Orbit propagation in the C++ facade now routes through the newer FFI orbit APIs, including explicit-center Kepler propagation and prepared-orbit handles.
- C++ coordinate transforms can now opt into non-default nutation/precession models without changing the existing default-path APIs.
- Build metadata and docs now register the new orbit/nutation examples and describe the orbit header as part of the public adapter surface.

## [0.1.0] - 2026-03-08

- Initial C++ bindings and FFI wrappers
- CMake build, headers under `include/`, examples in `examples/`, and basic tests in `tests/`
- Project README, CI scripts, and packaging metadata
