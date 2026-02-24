# C++ Examples

Build from the repository root:

```bash
cmake -S . -B build
cmake --build build
```

Run selected examples:

```bash
./build/siderust_demo
./build/coordinates_examples
./build/coordinate_systems_example
./build/solar_system_bodies_example
./build/altitude_events_example
./build/trackable_targets_example
./build/azimuth_lunar_phase_example
```

## Files

- `demo.cpp`: end-to-end extended walkthrough (time, typed coordinates, altitude/azimuth, trackables, ephemeris, lunar phase).
- `coordinates_examples.cpp`: typed coordinate construction and core conversion patterns.
- `coordinate_systems_example.cpp`: frame-tag traits and practical frame/horizontal transforms.
- `solar_system_bodies_example.cpp`: planet catalog constants, body-dispatch API, and ephemeris vectors.
- `altitude_events_example.cpp`: altitude windows/crossings/culminations for Sun, Moon, stars, ICRS directions, and `Target`.
- `trackable_targets_example.cpp`: polymorphic tracking with `Trackable`, `BodyTarget`, `StarTarget`, and `Target`.
- `azimuth_lunar_phase_example.cpp`: azimuth events/ranges plus lunar phase geometry, labels, and phase-event searches.
