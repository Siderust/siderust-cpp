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
| **Time** (`time.hpp`) | `JulianDate`, `MJD`, `UTC`, `Period` — value types with arithmetic and UTC round-trips |
| **Coordinates** (`coordinates.hpp`) | Modular typed API (`coordinates/{geodetic,spherical,cartesian,types}.hpp`) plus selective alias headers under `coordinates/types/{spherical,cartesian}/...` |
| **Frames & Centers** (`frames.hpp`, `centers.hpp`) | Compile-time frame/center tags, `FrameTraits`/`CenterTraits`, transform concept checks |
| **Bodies** (`bodies.hpp`) | `Star` (RAII, catalog + custom), `Planet` (8 planets), `ProperMotion`, `Orbit` |
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
#include <iomanip>

int main() {
    using namespace siderust;

    auto obs = ROQUE_DE_LOS_MUCHACHOS;
    auto jd  = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
    auto mjd = MJD::from_jd(jd);
    auto win = Period(mjd, mjd + qtty::Day(1.0));

    qtty::Degree sun_alt = sun::altitude_at(obs, mjd).to<qtty::Degree>();
    qtty::Degree sun_az  = sun::azimuth_at(obs, mjd);
    std::cout << "Sun alt=" << sun_alt.value() << " deg"
              << " az=" << sun_az.value() << " deg\n";

    Target fixed(279.23473, 38.78369); // Vega-like fixed ICRS target
    std::cout << "Target alt=" << fixed.altitude_at(obs, mjd).value() << " deg\n";

    auto nights = sun::below_threshold(obs, win, qtty::Degree(-18.0));
    std::cout << "Astronomical-night periods in next 24h: " << nights.size() << "\n";

    return 0;
}
```

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

# Run bundled examples
./siderust_demo
./coordinates_examples
./coordinate_systems_example
./solar_system_bodies_example
./altitude_events_example
./trackable_targets_example
./azimuth_lunar_phase_example

# Run tests
ctest --output-on-failure
```

---

## API Modules

- `siderust/siderust.hpp` — umbrella include for the full public API
- `siderust/time.hpp` — `UTC`, `JulianDate`, `MJD`, and `Period`
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
