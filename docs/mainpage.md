# siderust-cpp API Documentation

`siderust-cpp` is a header-only C++17 wrapper around the Rust-based `siderust`
astronomy engine. The API provides typed frames/centers, unit-safe quantities,
RAII wrappers for FFI handles, and exception-based error reporting.

## Modules

- `siderust/siderust.hpp`: umbrella include for the full public API.
- `siderust/time.hpp`: `UTC`, `JulianDate`, `MJD`, and `Period`.
- `siderust/coordinates.hpp`: modular coordinate API (`coordinates/geodetic.hpp`, `coordinates/spherical.hpp`, `coordinates/cartesian.hpp`, `coordinates/types.hpp`).
- `siderust/frames.hpp`: compile-time frame tags and transform traits.
- `siderust/centers.hpp`: compile-time center tags and center-shift traits.
- `siderust/bodies.hpp`: `Star`, `Planet`, and orbital/proper-motion types.
- `siderust/observatories.hpp`: known observatory locations and custom geodetic points.
- `siderust/altitude.hpp`: Sun/Moon/Star altitude queries and event search.
- `siderust/ephemeris.hpp`: VSOP87/ELP2000 position queries.

## Building This Documentation

If configured through CMake, generate docs with:

```bash
cmake -S . -B build
cmake --build build --target docs
```

Then open:

- `build/docs/doxygen/html/index.html`
