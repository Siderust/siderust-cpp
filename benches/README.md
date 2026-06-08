# siderust-cpp benchmarks

Google Benchmark executables that measure end-to-end altitude-range searches
through the C++ wrapper.

## Quick start

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DSIDERUST_CPP_BUILD_BENCHES=ON \
  -DSIDERUST_CPP_BUILD_TESTS=OFF
cmake --build build --target bench_night_periods bench_icrs_altitude_periods
./build/bench_night_periods
./build/bench_icrs_altitude_periods
```

Filter to a single case:

```bash
./build/bench_night_periods --benchmark_filter=sun_altitude_ranges/horizon/184
./build/bench_icrs_altitude_periods --benchmark_filter=icrs_altitude_ranges/airmass_30_75/184
```

## Performance target

A typical usage is:

```cpp
const auto nights = siderust::sun::altitude_ranges(
    geo, window, qtty::Degree(-90.0), qtty::Degree(horizon));
```

For a **6-month** window (184 days) at the geometric horizon (`0°`), this should
complete in **under 0.5 s** on a desktop CPU with a Release build. Check the
`sun_altitude_ranges/horizon/184` row in the benchmark output.

## What is measured

| Benchmark | API | Meaning |
|-----------|-----|---------|
| `sun_altitude_ranges/<horizon>/<days>` | `sun::altitude_ranges(geo, window, -90°, horizon)` | Night periods via the range query |
| `sun_below_threshold/<horizon>/<days>` | `sun::below_threshold(geo, window, horizon)` | Equivalent night-period fast path |
| `moon_above_threshold/<horizon>/<days>` | `moon::above_threshold(geo, window, horizon)` | Moon altitude threshold periods |
| `icrs_altitude_ranges/<band>/<days>` | `icrs_altitude::altitude_ranges(dir, geo, window, min_alt, max_alt)` | Periods when a fixed equatorial/ICRS direction is inside an altitude band |

Horizons: `horizon` (0°), `civil` (−6°), `nautical` (−12°), `astronomical` (−18°).

Windows: 30 days (1 month), 184 days (6 months), 365 days (1 year).

Site: Roque de los Muchachos (La Palma), matching the Rust `solar_altitude` bench.

The ICRS benchmark uses Vega's J2000 direction (`RA=279.2348°`, `Dec=38.7836°`)
and the bands `observable_0_90`, `science_20_80`, and `airmass_30_75`.
