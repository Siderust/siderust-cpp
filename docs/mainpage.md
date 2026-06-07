# siderust-cpp API Documentation

`siderust-cpp` is a **modern, header-only C++17 wrapper** for
[siderust](https://github.com/siderust/siderust) — a high-precision
astronomical computation engine written in Rust.

It exposes idiomatic C++ types (RAII, value semantics, `std::exception`
hierarchy) on top of the C FFI layer produced by `siderust-ffi` and
`tempoch-ffi`, so you can use the full power of siderust from any C++17
codebase without writing a single line of Rust.

---

## Features

| Module | What you get |
|--------|-------------|
| **Time** (`time.hpp`) | `UTC`, `CivilTime`, TT-default `JulianDate` / `MJD` / `Period`, plus explicit `Time<scale::S>` and `TimeContext` |
| **Coordinates** (`coordinates.hpp`) | Modular typed API (`coordinates/{geodetic,spherical,cartesian,types}.hpp`) plus selective alias headers under `coordinates/types/{spherical,cartesian}/...` |
| **Frames & Centers** (`frames.hpp`, `centers.hpp`) | Compile-time frame/center tags, `FrameTraits`/`CenterTraits`, transform concept checks |
| **Orbits** (`orbit.hpp`) | `KeplerianOrbit`, `MeanMotionOrbit`, `ConicOrbit`, `PreparedOrbit`, plus compatibility alias `Orbit` |
| **Bodies** (`bodies.hpp`) | `Star` (RAII, catalog + custom), `Planet` (8 planets), `ProperMotion`, planet orbit data |
| **Observatories** (`observatories.hpp`) | Named sites: Roque de los Muchachos, Paranal, Mauna Kea, La Silla |
| **Altitude** (`altitude.hpp`) | Sun / Moon / Star / ICRS altitude: instant, above/below threshold, crossings, culminations |
| **Azimuth** (`azimuth.hpp`) | Sun / Moon / Star / ICRS azimuth: instant, crossings, extrema, range windows |
| **Targets** (`trackable.hpp`, `target.hpp`, `body_target.hpp`, `star_target.hpp`) | Polymorphic target tracking across bodies, stars, and fixed ICRS directions |
| **Lunar Phase** (`lunar_phase.hpp`) | Moon phase geometry, labels, principal phase events, illumination windows |
| **Ephemeris** (`ephemeris.hpp`) | VSOP87 Sun/Earth positions, ELP2000 Moon position |

---

## Quick Start

```cpp
#include <siderust/siderust.hpp>
#include <iostream>

int main() {
    using namespace siderust;

    auto obs = ROQUE_DE_LOS_MUCHACHOS;
    auto mjd = MJD::from_utc({2026, 7, 15, 22, 0, 0});
    Period win(mjd, mjd + qtty::Day(1.0));

    qtty::Degree sun_alt = sun::altitude_at(obs, mjd).to<qtty::Degree>();
    qtty::Degree sun_az = sun::azimuth_at(obs, mjd);
    std::cout << "Sun alt=" << sun_alt.value() << " deg"
              << " az=" << sun_az.value() << " deg\n";

    Target fixed(279.23473, 38.78369);
    std::cout << "Target alt=" << fixed.altitude_at(obs, mjd).value() << " deg\n";

    auto nights = sun::below_threshold(obs, win, qtty::Degree(-18.0));
    std::cout << "Astronomical-night periods in next 24h: " << nights.size() << "\n";
}
```

## Altitude Search Controls

Sun and Moon altitude searches use Auto mode by default. Explicit algorithm control is additive:

```cpp
SearchOptions opts;
opts.with_algorithm(CrossingAlgorithm::ChebyshevRoots);

auto sunrise = sun::crossings(obs, win, qtty::Degree(0.0), opts);
auto moon_up = moon::above_threshold(obs, win, qtty::Degree(0.0), opts);
```

Use `CrossingAlgorithm::ScanBrent` to force the legacy scan+Brent path.

---

## Architecture

```
┌──────────────┐
│  C++ user    │   #include <siderust/siderust.hpp>
│  code        │
└──────┬───────┘
       │  header-only (inline)
┌──────▼───────┐
│ siderust-cpp │   C++17 types, RAII, exceptions
│ (headers)    │
└──────┬───────┘
       │  extern "C" calls
┌──────▼───────┐  ┌──────────────┐
│ siderust-ffi │──│ tempoch-ffi  │   C ABI (cbindgen-generated)
│ (.so/.dylib) │  │ (.so/.dylib) │
└──────┬───────┘  └──────┬───────┘
       │                 │
┌──────▼─────────────────▼──┐
│       siderust (Rust)     │
│  coordinates · altitude   │
│  bodies · ephemeris       │
│  tempoch · affn · qtty    │
└───────────────────────────┘
```

---

## Prerequisites

- C++17 compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.15+
- Rust toolchain (`cargo`) — FFI libraries are built automatically

---

## Building

```bash
# Clone with submodules
git clone --recurse-submodules <url>
cd siderust-cpp

mkdir build && cd build
cmake ..
cmake --build .

# Run examples
./01_basic_coordinates_example
./10_time_periods_example
./14_nutation_models_example
./15_orbit_models_example

# Run tests
ctest --output-on-failure
```

---

## API Modules

- `siderust/siderust.hpp` — umbrella include for the full public API
- `siderust/time.hpp` — `UTC`, `CivilTime`, TT-default `JulianDate` / `MJD` / `Period`, explicit `Time<scale::S>`, and `TimeContext`

## Time API

- Default astronomy-facing code should use `JulianDate`, `MJD`, and `Period`, all pinned to TT.
- Civil construction is available directly through `JulianDate::from_utc(...)` and `MJD::from_utc(...)`.
- Advanced mixed-scale work stays explicit with `Time<scale::S>` and named aliases such as `UT1JulianDate`.
- `siderust/coordinates.hpp` — modular coordinate API (`coordinates/geodetic.hpp`, `coordinates/spherical.hpp`, `coordinates/cartesian.hpp`, `coordinates/types.hpp`)
- \ref coordinate_types — quick reference for coordinate types and their main methods
- `siderust/frames.hpp` — compile-time frame tags and transform traits
- `siderust/centers.hpp` — compile-time center tags and center-shift traits
- `siderust/bodies.hpp` — `Star`, `Planet`, and orbital / proper-motion types
- `siderust/observatories.hpp` — known observatory locations and custom geodetic points
- `siderust/altitude.hpp` — Sun / Moon / Star altitude queries and event search
- `siderust/azimuth.hpp` — azimuth queries, crossings, extrema, and azimuth ranges
- `siderust/trackable.hpp`, `siderust/target.hpp`, `siderust/body_target.hpp`, `siderust/star_target.hpp` — target abstractions and polymorphic tracking
- `siderust/lunar_phase.hpp` — moon phase geometry, labels, phase events, illumination windows
- `siderust/ephemeris.hpp` — VSOP87 / ELP2000 position queries

---

## Building This Documentation

If Doxygen is installed, CMake exposes a `docs` target:

```bash
cmake -S . -B build
cmake --build build --target docs
```

Then open:

- `build/docs/doxygen/html/index.html`
