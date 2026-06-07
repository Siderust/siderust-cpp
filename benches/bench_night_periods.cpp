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

struct AlgorithmCase {
  const char *label;
  CrossingAlgorithm algorithm;
};

constexpr HorizonCase kHorizons[] = {
    {"horizon", qtty::Degree(0.0)},
    {"civil", qtty::Degree(-6.0)},
    {"nautical", qtty::Degree(-12.0)},
    {"astronomical", qtty::Degree(-18.0)},
};

constexpr AlgorithmCase kAlgorithms[] = {
    {"auto", CrossingAlgorithm::Auto},
    {"scan_brent", CrossingAlgorithm::ScanBrent},
    {"chebyshev_roots", CrossingAlgorithm::ChebyshevRoots},
};

Period<TT, MJD> window_from_days(const Time<TT, MJD> &start, int days) {
  return Period<TT, MJD>(start, start + qtty::Day(static_cast<double>(days)));
}

SearchOptions search_options(CrossingAlgorithm algorithm) {
  SearchOptions opts;
  opts.with_algorithm(algorithm);
  return opts;
}

void bench_sun_altitude_periods(benchmark::State &state, qtty::Degree horizon,
                                CrossingAlgorithm algorithm) {
  const auto geo = ROQUE_DE_LOS_MUCHACHOS();
  const auto start = Time<TT, MJD>::from_utc({2026, 1, 1, 0, 0, 0});
  const auto window = window_from_days(start, static_cast<int>(state.range(0)));
  const auto opts = search_options(algorithm);

  for (auto _ : state) {
    (void)_; // avoid "unused variable" warning
    const auto nights = sun::altitude_periods(geo, window, qtty::Degree(-90.0), horizon, opts);
    benchmark::DoNotOptimize(nights.data());
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(state.iterations());
  state.counters["days"] = static_cast<double>(state.range(0));
}

void bench_sun_below_threshold(benchmark::State &state, qtty::Degree horizon,
                               CrossingAlgorithm algorithm) {
  const auto geo = ROQUE_DE_LOS_MUCHACHOS();
  const auto start = Time<TT, MJD>::from_utc({2026, 1, 1, 0, 0, 0});
  const auto window = window_from_days(start, static_cast<int>(state.range(0)));
  const auto opts = search_options(algorithm);

  for (auto _ : state) {
    (void)_; // avoid "unused variable" warning
    const auto nights = sun::below_threshold(geo, window, horizon, opts);
    benchmark::DoNotOptimize(nights.data());
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(state.iterations());
  state.counters["days"] = static_cast<double>(state.range(0));
}

void bench_moon_above_threshold(benchmark::State &state, qtty::Degree horizon,
                                CrossingAlgorithm algorithm) {
  const auto geo = ROQUE_DE_LOS_MUCHACHOS();
  const auto start = Time<TT, MJD>::from_utc({2026, 1, 1, 0, 0, 0});
  const auto window = window_from_days(start, static_cast<int>(state.range(0)));
  const auto opts = search_options(algorithm);

  for (auto _ : state) {
    (void)_; // avoid "unused variable" warning
    const auto periods = moon::above_threshold(geo, window, horizon, opts);
    benchmark::DoNotOptimize(periods.data());
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(state.iterations());
  state.counters["days"] = static_cast<double>(state.range(0));
}

void register_horizon_benchmarks(const char *api,
                                 void (*fn)(benchmark::State &, qtty::Degree, CrossingAlgorithm)) {
  for (const auto &horizon : kHorizons) {
    for (const auto &algorithm : kAlgorithms) {
      const std::string name =
          std::string(api) + "/" + horizon.label + "/" + algorithm.label;
      benchmark::RegisterBenchmark(name.c_str(), fn, horizon.threshold, algorithm.algorithm)
          ->Arg(30)
          ->Arg(184)
          ->Arg(365)
          ->Unit(benchmark::kMillisecond);
    }
  }
}

} // namespace

int main(int argc, char **argv) {
  register_horizon_benchmarks("sun_altitude_periods", bench_sun_altitude_periods);
  register_horizon_benchmarks("sun_below_threshold", bench_sun_below_threshold);
  register_horizon_benchmarks("moon_above_threshold", bench_moon_above_threshold);

  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
    return 1;
  }

  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}
