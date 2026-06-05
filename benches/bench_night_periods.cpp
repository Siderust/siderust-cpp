// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// Night-period benchmarks for siderust-cpp.
///
/// Typical usage:
///   const auto nights = siderust::sun::altitude_periods(
///       geo, window, qtty::Degree(-90.0), qtty::Degree(horizon));
///
/// Target: a 6-month window completes in under 0.5 s on a typical desktop CPU
/// (Release build, -O2 or better).

#include <benchmark/benchmark.h>
#include <siderust/siderust.hpp>

#include <string>

using namespace siderust;
using namespace qtty::literals;

namespace {

struct HorizonCase {
  const char *label;
  qtty::Degree threshold;
};

constexpr HorizonCase kHorizons[] = {
    {"horizon", qtty::Degree(0.0)},
    {"civil", qtty::Degree(-6.0)},
    {"nautical", qtty::Degree(-12.0)},
    {"astronomical", qtty::Degree(-18.0)},
};

Period window_from_days(const ModifiedJulianDate &start, int days) {
  return Period(start, start + qtty::Day(static_cast<double>(days)));
}

void bench_altitude_periods(benchmark::State &state, qtty::Degree horizon) {
  const auto geo = ROQUE_DE_LOS_MUCHACHOS();
  const auto start = ModifiedJulianDate::from_utc({2026, 1, 1, 0, 0, 0});
  const auto window = window_from_days(start, static_cast<int>(state.range(0)));

  for (auto _ : state) {
    const auto nights = sun::altitude_periods(geo, window, qtty::Degree(-90.0), horizon);
    benchmark::DoNotOptimize(nights.data());
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(state.iterations());
  state.counters["days"] = static_cast<double>(state.range(0));
}

void bench_below_threshold(benchmark::State &state, qtty::Degree horizon) {
  const auto geo = ROQUE_DE_LOS_MUCHACHOS();
  const auto start = ModifiedJulianDate::from_utc({2026, 1, 1, 0, 0, 0});
  const auto window = window_from_days(start, static_cast<int>(state.range(0)));

  for (auto _ : state) {
    const auto nights = sun::below_threshold(geo, window, horizon);
    benchmark::DoNotOptimize(nights.data());
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(state.iterations());
  state.counters["days"] = static_cast<double>(state.range(0));
}

void register_horizon_benchmarks(const char *api, void (*fn)(benchmark::State &, qtty::Degree)) {
  for (const auto &horizon : kHorizons) {
    const std::string name = std::string(api) + "/" + horizon.label;
    benchmark::RegisterBenchmark(name.c_str(), fn, horizon.threshold)
        ->Arg(30)
        ->Arg(184)
        ->Arg(365)
        ->Unit(benchmark::kMillisecond);
  }
}

} // namespace

int main(int argc, char **argv) {
  register_horizon_benchmarks("altitude_periods", bench_altitude_periods);
  register_horizon_benchmarks("below_threshold", bench_below_threshold);

  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
    return 1;
  }

  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}
