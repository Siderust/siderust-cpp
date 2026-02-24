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
| **Ephemeris** (`ephemeris.hpp`) | VSOP87 Sun/Earth positions, ELP2000 Moon position |

---

## Quick Start

```cpp
#include <siderust/siderust.hpp>
#include <iostream>
#include <iomanip>

int main() {
    using namespace siderust;
    using namespace qtty::literals;

    auto obs = ROQUE_DE_LOS_MUCHACHOS;
    auto jd  = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
    auto mjd = MJD::from_jd(jd);

    // Sun altitude at the observatory
    qtty::Radian alt = sun::altitude_at(obs, mjd);
    std::cout << std::fixed << std::setprecision(4) << "Sun altitude: " << alt << " rad\n";

    // Star from built-in catalog
    const auto& vega    = VEGA;
    qtty::Radian star_alt = star_altitude::altitude_at(vega, obs, mjd);
    std::cout << "Vega altitude: " << star_alt << " rad\n";

    // Astronomical night periods (twilight < -18°)
    auto nights = sun::below_threshold(obs, mjd, mjd + 1.0, -18.0_deg);
    for (auto& p : nights)
        std::cout << "Night: MJD " << p.start() << " – " << p.end() << "\n";

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
