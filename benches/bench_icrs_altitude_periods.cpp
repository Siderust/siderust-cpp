// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// Fixed-equatorial-direction altitude-period benchmarks for siderust-cpp.
///
/// Typical usage:
///   const auto periods = siderust::icrs_altitude::altitude_periods(
///       vega_icrs, geo, window, qtty::Degree(min_alt), qtty::Degree(max_alt));

#include <benchmark/benchmark.h>
#include <siderust/siderust.hpp>

#include <string>

using namespace siderust;
using namespace qtty::literals;

namespace {

struct AltitudeBandCase {
  const char *label;
  qtty::Degree min_alt;
  qtty::Degree max_alt;
};

constexpr AltitudeBandCase kAltitudeBands[] = {
    {"observable_0_90", qtty::Degree(0.0), qtty::Degree(90.0)},
    {"science_20_80", qtty::Degree(20.0), qtty::Degree(80.0)},
    {"airmass_30_75", qtty::Degree(30.0), qtty::Degree(75.0)},
};

Period<TT, MJD> window_from_days(const Time<TT, MJD> &start, int days) {
  return Period<TT, MJD>(start, start + qtty::Day(static_cast<double>(days)));
}

void bench_icrs_altitude_periods(benchmark::State &state, qtty::Degree min_alt,
                                 qtty::Degree max_alt) {
  const auto geo = ROQUE_DE_LOS_MUCHACHOS();
  const spherical::direction::ICRS vega_icrs(279.2348_deg, 38.7836_deg);
  const auto start = Time<TT, MJD>::from_utc({2026, 1, 1, 0, 0, 0});
  const auto window = window_from_days(start, static_cast<int>(state.range(0)));

  for (auto _ : state) {
    (void)_;
    const auto periods = icrs_altitude::altitude_periods(vega_icrs, geo, window, min_alt, max_alt);
    benchmark::DoNotOptimize(periods.data());
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(state.iterations());
  state.counters["days"] = static_cast<double>(state.range(0));
  state.counters["min_alt_deg"] = min_alt.value();
  state.counters["max_alt_deg"] = max_alt.value();
}

void register_altitude_band_benchmarks() {
  for (const auto &band : kAltitudeBands) {
    const std::string name = std::string("icrs_altitude_periods/") + band.label;
    benchmark::RegisterBenchmark(name.c_str(), bench_icrs_altitude_periods, band.min_alt,
                                 band.max_alt)
        ->Arg(30)
        ->Arg(184)
        ->Arg(365)
        ->Unit(benchmark::kMillisecond);
  }
}

} // namespace

int main(int argc, char **argv) {
  register_altitude_band_benchmarks();

  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
    return 1;
  }

  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}
