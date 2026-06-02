// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

#include <cmath>
#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;

namespace {

// Vallado SGP4 test satellite NORAD 5 — same TLE as Rust example 17.
static constexpr const char *L1 =
    "1 00005U 58002B   00179.78495062  .00000023  00000-0  28098-4 0  4753";
static constexpr const char *L2 =
    "2 00005  34.2682 348.7242 1859667 331.7664  19.3264 10.82419157413667";

TEST(Tle, ParseValidTle) {
  auto t = tle::Tle::parse(L1, L2);
  EXPECT_EQ(t.norad_id(), 5u);
}

TEST(Tle, ParseInvalidThrows) {
  EXPECT_THROW(tle::Tle::parse("bad line", "bad line"), InvalidArgumentError);
}

TEST(Sgp4, EpochJdInReasonableRange) {
  auto t = tle::Tle::parse(L1, L2);
  auto prop = sgp4::Propagator(t);
  // TLE epoch is year 2000 (JD ~2451545 ± a year).
  double jd = prop.epoch_jd_utc();
  EXPECT_GT(jd, 2451000.0);
  EXPECT_LT(jd, 2452000.0);
}

TEST(Sgp4, PropagateAtEpoch) {
  auto t = tle::Tle::parse(L1, L2);
  auto prop = sgp4::Propagator(t);
  auto s = prop.propagate_at(prop.epoch_jd_utc());
  // Position magnitude should be a plausible LEO altitude (3000–10000 km).
  double r =
      std::sqrt(s.pos_km[0] * s.pos_km[0] + s.pos_km[1] * s.pos_km[1] + s.pos_km[2] * s.pos_km[2]);
  EXPECT_GT(r, 3'000.0);
  EXPECT_LT(r, 10'000.0);
}

TEST(Sgp4, PropagateOffset) {
  auto t = tle::Tle::parse(L1, L2);
  auto prop = sgp4::Propagator(t);
  // States at different epochs must differ.
  auto s0 = prop.propagate_at(prop.epoch_jd_utc());
  auto s1 = prop.propagate_at(prop.epoch_jd_utc() + 360.0 / 1440.0);
  EXPECT_NE(s0.pos_km[0], s1.pos_km[0]);
}

TEST(Sgp4, DefaultGravityModelIsWgs72) {
  auto t = tle::Tle::parse(L1, L2);
  auto prop = sgp4::Propagator(t);
  EXPECT_EQ(prop.gravity_model(), 0);
}

TEST(Sgp4, MoveSemantics) {
  auto t = tle::Tle::parse(L1, L2);
  auto prop1 = sgp4::Propagator(t);
  auto prop2 = std::move(prop1);
  // After move, prop2 is valid.
  double jd = prop2.epoch_jd_utc();
  EXPECT_GT(jd, 2451000.0);
}

} // namespace
