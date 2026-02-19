# C++ Examples

Build from the repository root:

```bash
cmake -S . -B build-make
cmake --build build-make
```

Run selected examples:

```bash
./build-make/demo
./build-make/coordinates_examples
./build-make/coordinate_systems_example
./build-make/solar_system_bodies_example
./build-make/altitude_events_example
```

## Files

- `demo.cpp`: broad API walkthrough.
- `coordinates_examples.cpp`: typed coordinate creation and frame transforms.
- `coordinate_systems_example.cpp`: coordinate systems + direction/position transforms in one place.
- `solar_system_bodies_example.cpp`: ephemeris vectors and static planet catalog data.
- `altitude_events_example.cpp`: altitude periods, crossings, and culminations for Sun, Moon, VEGA, and fixed ICRS directions.
